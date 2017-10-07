#include <tm4c1294ncpdt/Clocking/Clocking_TivaTM4C1294NCPDT.h>
#include "inc/hw_ints.h"
#include "inc/hw_timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_types.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "inc/hw_nvic.h"

const uint32_t DefaultCpuFreq = 120000000;

static void (*SysTickCbFuncs[8])(uint32_t timeMs);

static const uint16_t SystickIntPriority = 0x80;
static const uint32_t SystickHz = 1000;
static const uint32_t SystickMs = 1000 / SystickHz;

static uint32_t F_CPU;
static volatile uint32_t milliseconds = 0;

static void SysTickIntHandler();

void initSystemClocks()
{ 
  F_CPU = DefaultCpuFreq;
  MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_CFG_VCO_480), F_CPU);
  
  //
  //  SysTick is used for delay() and delayMicroseconds()
  //

  MAP_SysTickPeriodSet(F_CPU / SystickHz);
  SysTickIntRegister(SysTickIntHandler);
  MAP_SysTickEnable();
  MAP_IntPrioritySet(FAULT_SYSTICK, SystickIntPriority);
  MAP_SysTickIntEnable();
  MAP_IntMasterEnable();

  // PIOSC is used during Deep Sleep mode for wakeup
  MAP_SysCtlPIOSCCalibrate(SYSCTL_PIOSC_CAL_FACT);  // Factory-supplied calibration used
}

uint32_t setCpuClockFreq(uint32_t newFrequency)
{
  F_CPU = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_CFG_VCO_480), newFrequency);
  
  return(F_CPU);
}

uint32_t getCpuClockFreq()
{
  return(F_CPU);
}

uint32_t micros(void)
{
  //micros so far + system ticks since last millisecond rollover (System clock ticks downward) / ticks it takes for 1 microsecond. 
	return (milliseconds * 1000) + ( ((F_CPU / SystickHz) - MAP_SysTickValueGet()) / (F_CPU/1000000));
}

uint32_t millis(void)
{
	return milliseconds;
}

void delayMicroseconds(uint32_t micros)
{
	// Systick timer rolls over every 1000000/SYSTICKHZ microseconds 
	if (micros > (1000000UL / SystickHz - 1)) {
		delay(micros / 1000);  // delay milliseconds
		micros = micros % 1000;     // handle remainder of delay
	};

	// 24 bit timer - mask off undefined bits
	unsigned long startTime = HWREG(NVIC_ST_CURRENT) & NVIC_ST_CURRENT_M;

	unsigned long ticks = (unsigned long)micros * (F_CPU/1000000UL);
	volatile unsigned long elapsedTime;

	if (ticks > startTime) {
		ticks = (ticks + (NVIC_ST_CURRENT_M - (unsigned long)F_CPU / SystickHz)) & NVIC_ST_CURRENT_M;
	}

	do {
		elapsedTime = (startTime-(HWREG(NVIC_ST_CURRENT) & NVIC_ST_CURRENT_M )) & NVIC_ST_CURRENT_M;
	} while(elapsedTime <= ticks);
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

	uint8_t i;
	for(i = 0; i < 8; i++)
	{
		if(SysTickCbFuncs[i])
		{
			SysTickCbFuncs[i](SystickMs);
		}
	}
}

void registerSysTickCb(void (*userFunc)(uint32_t))
{
	uint8_t i;
	for(i = 0; i < 8; i++)
	{
		if(!SysTickCbFuncs[i])
		{
			SysTickCbFuncs[i] = userFunc;
			break;
		}
	}
}
