/*
 * Clocking_MSP432P401R.cpp
 *
 *  Created on: Oct 14, 2017
 *      Author: drue
 */

#include "Clocking_MSP432P401R.h"
#include "../msp432Ware/driverlib/driverlib.h"
#include "../msp432Ware/inc/msp432p401r.h"
#include "supportingUtilities/Debug.h"
#include "supportingUtilities/RoveUtilities.h"

static void SysTickIntHandler();
static bool needChangePowerStateFirst(uint32_t desiredCpuFreq, uint32_t desiredPeriphFreq);
static void selectOptimalPowerState(uint32_t cpuFreq, uint32_t periphFreq);
static uint32_t setClockFreq(uint32_t newFrequency, uint32_t clockToSet);

static const uint16_t SystickIntPriority = 0x80;
static uint32_t milliseconds = 0;
static uint16_t SystickHz = 1000;

#define HFXTCLK_MINIMUM (EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ / 128) //lowest clock divider is 128, so this is the minimum freq we can get on HFXT
#define DCOCLK_MINIMUM (CS_getDCOFrequency() / 128)

#define CpuMaxFreq 48000000
#define PeriphMaxFreq 24000000
#define DCOMaxFreq 48000000

void initSystemClocks()
{
  // Configuring pins for peripheral/crystal usage
  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);

  //set the internal reference for what the frequency fo the external crystals are
  CS_setExternalClockSourceFrequency(EXTERNAL_LOW_FREQUENCY_CRYSTAL_FREQ, EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ);

  // Starting HFXT and LFXT in non-bypass mode without a timeout. Before we start we have to change VCORE to 1 to support the 48MHz frequency
  MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
  CS_startHFXT(false);
  CS_startLFXT(CS_LFXT_DRIVE3);

  // Initializing clocks
  MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1); //main clock set to use HF external clock, 48Mhz which is its max. Used by CPU
  MAP_CS_initClockSignal(CS_HSMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_2); //sub-main clock set to use HF external clock, 24Mhz which is its max. Used by periphs.
                                                                           //low-speed submain uses same clock as sub-main. Used by periphs
                                                                           //aux and backup clocks default to LF external clock.

  MAP_CS_setDCOFrequency(3000000); //set DCO to default of 3 Mhz

  //
  //  SysTick is used for delay() and delayMicroseconds() and micros and millis
  //
  MAP_SysTick_enableModule();
  MAP_SysTick_setPeriod(CS_getMCLK() / 1000); //uses MCLK. Hz (ticks/s) * 1 ms = ticks needed to make it time out in 1 millisecond
  MAP_SysTick_enableInterrupt();
  MAP_SysTick_registerInterrupt(SysTickIntHandler);
  MAP_Interrupt_setPriority(FAULT_SYSTICK, SystickIntPriority);
  MAP_Interrupt_enableMaster();

}

uint32_t setCpuClockFreq(uint32_t newFrequency)
{
  if(newFrequency > CpuMaxFreq)
  {
    newFrequency = CpuMaxFreq;
  }
  return setClockFreq(newFrequency, CS_MCLK);
}

uint32_t setPeriphClockFreq(uint32_t newFrequency)
{
  if(newFrequency > PeriphMaxFreq)
  {
    newFrequency = PeriphMaxFreq;
  }
  return setClockFreq(newFrequency, CS_HSMCLK);
}

uint32_t setProgrammableClockSourceFreq(uint32_t newFrequency)
{
  if(newFrequency > DCOMaxFreq)
  {
    newFrequency = DCOMaxFreq;
  }
  MAP_CS_setDCOFrequency (newFrequency);
  return MAP_CS_getDCOFrequency();
}

uint32_t getCpuClockFreq()
{
  return(CS_getMCLK());
}

uint32_t getPeriphClockFreq()
{
  return(CS_getHSMCLK());
}

uint32_t micros(void)
{
  //micros so far + system ticks since last millisecond rollover (System clock ticks downward) / ticks it takes for 1 microsecond.
  return (milliseconds * 1000) + ( ((CS_getMCLK() / SystickHz) - MAP_SysTick_getValue()) / (((float)CS_getMCLK())/1000000.0));
}

uint32_t millis(void)
{
  return milliseconds;
}

void delayMicroseconds(uint32_t microsToDelay)
{
  // Systick timer rolls over every 1000000/SYSTICKHZ microseconds
  if (microsToDelay > (1000000UL / SystickHz - 1)) {
    delay(microsToDelay / 1000);  // delay milliseconds
    microsToDelay = microsToDelay % 1000;     // handle remainder of delay
  };

  uint32_t timeStart = micros();

  while(microsToDelay < (micros() - timeStart));
}

void delay(uint32_t millis)
{
  uint32_t i;
  for(i = 0; i < millis*2; i++)
  {
    delayMicroseconds(500);
  }
}

static void SysTickIntHandler(void)
{
  milliseconds++;
}

static bool needChangePowerStateFirst(uint32_t desiredCpuFreq, uint32_t desiredPeriphFreq)
{
  uint32_t presentCpuFreq = MAP_CS_getMCLK();
  uint32_t presentPeriphFreq = MAP_CS_getHSMCLK();

  //If either clock is operating in the range of minimum power mode but is about to transition to the range used by high power mode, then
  //we need to change power states first.
  //If either clock is operating in the range of maximum power mode already, then the clocks are free to change before we try moving to a different
  //power level since high power mode can support all frequencies.
  if((desiredCpuFreq > MinVoltageMode_CpuMaxFreq && presentCpuFreq <= MinVoltageMode_CpuMaxFreq) || (desiredPeriphFreq > MinVoltageMode_PeriphMaxFreq && presentPeriphFreq <= MinVoltageMode_PeriphMaxFreq))
  {
    return true;
  }
  else
  {
    return false;
  }
}

static void selectOptimalPowerState(uint32_t cpuFreq, uint32_t periphFreq)
{
  if(cpuFreq <= MinVoltageMode_CpuMaxFreq && periphFreq <= MinVoltageMode_PeriphMaxFreq)
  {
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE0);
  }
  else
  {
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
  }
}

static uint32_t setClockFreq(uint32_t newFrequency, uint32_t clockToSet)
{
  uint32_t mainClockSourceFreq;
  uint32_t mainClockSource;
  uint32_t hfxtComputedFreq;
  uint32_t dcoComputedFreq;
  uint32_t firmwareDivisor;
  uint32_t cpuFreq;
  uint32_t periphFreq;
  uint8_t literalDivisor;
  uint8_t i;

  //figure out what clock source we need to be using for the requested range
  if(newFrequency < HFXTCLK_MINIMUM && newFrequency < DCOCLK_MINIMUM)
  {
    mainClockSourceFreq = EXTERNAL_LOW_FREQUENCY_CRYSTAL_FREQ;
    mainClockSource = CS_LFXTCLK_SELECT;
  }
  else if(newFrequency < HFXTCLK_MINIMUM && newFrequency >= DCOCLK_MINIMUM)
  {
    mainClockSourceFreq = CS_getDCOFrequency();
    mainClockSource = CS_DCOCLK_SELECT;
  }
  else if(newFrequency >= HFXTCLK_MINIMUM && newFrequency < DCOCLK_MINIMUM)
  {
    mainClockSourceFreq = EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ;
    mainClockSource = CS_HFXTCLK_SELECT;
  }
  else //either DCO or HFXT will work. Let's figure out which would be more accurate
  {
    mainClockSourceFreq = CS_getDCOFrequency();
    for(i = 1; i < 128; i *= 2)
    {
      int temp = mainClockSourceFreq / i;
      if(temp <= newFrequency)
      {
        dcoComputedFreq = temp;
        break;
      }
    }

    mainClockSourceFreq = EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ;
    for(i = 1; i < 128; i *= 2)
    {
      int temp = mainClockSourceFreq / i;
      if(temp <= newFrequency)
      {
        hfxtComputedFreq = temp;
        break;
      }
    }

    if(abs(hfxtComputedFreq - newFrequency) > abs(dcoComputedFreq - newFrequency))
    {
      mainClockSourceFreq = CS_getDCOFrequency();
      mainClockSource = CS_DCOCLK_SELECT;
    }
    else
    {
      mainClockSourceFreq = EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ;
      mainClockSource = CS_HFXTCLK_SELECT;
    }
  }

  //figure out which divisor we need to use for the requested range
  if(newFrequency >= mainClockSourceFreq)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_1;
    literalDivisor = 1;
  }
  else if(newFrequency >= mainClockSourceFreq / 2)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_2;
    literalDivisor = 2;
  }
  else if(newFrequency >= mainClockSourceFreq / 4)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_4;
    literalDivisor = 4;
  }
  else if(newFrequency >= mainClockSourceFreq / 8)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_8;
    literalDivisor = 8;
  }
  else if(newFrequency >= mainClockSourceFreq / 16)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_16;
    literalDivisor = 16;
  }
  else if(newFrequency >= mainClockSourceFreq / 32)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_32;
    literalDivisor = 32;
  }
  else if(newFrequency >= mainClockSourceFreq / 64)
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_64;
    literalDivisor = 64;
  }
  else
  {
    firmwareDivisor = CS_CLOCK_DIVIDER_128;
    literalDivisor = 128;
  }

  //Compute the desired and achievable new frequencies
  if(clockToSet == CS_MCLK)
  {
    cpuFreq = mainClockSourceFreq / literalDivisor;
    periphFreq = MAP_CS_getHSMCLK();
  }
  else if(clockToSet == CS_HSMCLK)
  {
    cpuFreq = MAP_CS_getMCLK();
    periphFreq = mainClockSourceFreq / literalDivisor;
  }

  //configure clock to new setting, but be careful to make sure the power settings
  //will allow it. Either way, optimize power settings at the same time.
  if(needChangePowerStateFirst(cpuFreq,periphFreq))
  {
    selectOptimalPowerState(cpuFreq, periphFreq);
    MAP_CS_initClockSignal(clockToSet, mainClockSource, firmwareDivisor);
  }
  else
  {
    MAP_CS_initClockSignal(clockToSet, mainClockSource, firmwareDivisor);
    selectOptimalPowerState(cpuFreq, periphFreq);
  }

  for(i = 0; i < 100; i++);

  if(clockToSet == CS_MCLK)
  {
    MAP_SysTick_setPeriod(CS_getMCLK() / 1000); //uses MCLK. Hz (ticks/s) * 1 ms = ticks needed to make it time out in 1 millisecond
    return CS_getMCLK();
  }
  else if(clockToSet == CS_HSMCLK)
  {
    return CS_getHSMCLK();
  }
  else
  {
    debugFault("SetClockFreq error: Clock not supported");
    return 0;
  }
}
