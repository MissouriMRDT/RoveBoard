#include "../Clocking/Clocking_MSP432P401R.h"
#include "supportingUtilities/Debug.h"
#include "supportingUtilities/RoveUtilities.h"
#include "RovePwmWrite_MSP432P401R.h"
#include "../RovePinMap_MSP432P401R.h"
#include "../msp432Ware/driverlib/driverlib.h"

static const uint8_t NotARegister = 0;
static const int PeriodMaxLoad = 65535;
static const int DefaultPeriodUs = 2401;

typedef struct TimerAPwmConfig
{
  uint_fast16_t clockSource;
  uint_fast16_t clockSourceDivider;
  uint_fast16_t timerPeriod;

  TimerAPwmConfig()
  {
    //Optimization routine needs a starting setpoint, so we hardcode in a fairly optimal default state.
    //SMCLK has freq of 24Mhz on startup.
    //period_us = (load * 1000000) / (SOURCE_FREQ / DIVIDER)
    //plugging it in, these default settings gives us 2041 in microseconds.
    clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timerPeriod = 49000;
  }

  uint32_t getTimerPeriodUs()
  {
    return (1000000 * timerPeriod) / (getClockSourceFreq() / clockSourceDivider);
  }

  uint32_t pulseWidthUsToTimerTicks(uint32_t pulseWidthUs)
  {
    float percentDuty = ((float)pulseWidthUs) / ((float)getTimerPeriodUs());

    return percentDuty * ((float)timerPeriod);
  }

  uint32_t timerPeriodUsToTimerPeriodTicks(uint32_t timerPeriodUs)
  {
    return (timerPeriodUs/ 1000000) * (getClockSourceFreq() / clockSourceDivider);
  }

  uint32_t getClockSourceFreq()
  {
    uint32_t clockSourceFreq;

    if(clockSource == TIMER_A_CLOCKSOURCE_ACLK)
    {
      clockSourceFreq = CS_getACLK();
    }
    else if(clockSource == TIMER_A_CLOCKSOURCE_SMCLK)
    {
      clockSourceFreq = CS_getSMCLK();
    }

    return clockSourceFreq;
  }

}TimerAPwmConfig;

static TimerAPwmConfig pwm0Config, pwm1Config, pwm2Config, pwm3Config;

static uint32_t getTimerPeriodTicks(const uint32_t pulsePeriod_us, const uint32_t clockFreq, const uint8_t clockDivisor);
static uint32_t getOptimalDivisorGoingFaster(const uint32_t pulsePeriod_us, const uint32_t clockFreq, const uint8_t clockDivisor);
static uint32_t getOptimalDivisorGoingFaster(const uint32_t pulsePeriod_us, const uint32_t clockFreq);
static uint32_t getOptimalDivisorGoingSlower(const uint32_t pulsePeriod_us, const uint32_t clockFreq, const uint8_t clockDivisor);
static uint32_t getOptimalDivisorGoingSlower(const uint32_t pulsePeriod_us, const uint32_t clockFreq);
static bool getOptimalClockSettings(uint32_t pulsePeriod_us, TimerAPwmConfig *currentConfig, uint32_t *clockSource, uint8_t *clockDivisor);


////////////////////////constant static lookup tables//////////////

//look up table to convert the enum to the desired value for configuring the gnerator
static uint32_t alignmentToOutputMode[] =
{
  TIMER_A_OUTPUTMODE_RESET_SET,     //LeftAligned
  TIMER_A_OUTPUTMODE_SET_RESET,     //RightAligned
};

//Returns which capture/compare register is associated with the pin
static uint8_t pinToCCRegister[] =
{
    NotARegister,              //dummy
    NotARegister,              //1           P10.1       digital, UCB3CLK
    NotARegister,              //2           P10.2       digital, UCB3SIMO, UCB3SDA
    NotARegister,              //3           P10.3       digital, UCB3SOMI, UCB3SCL
    NotARegister,              //4           P1.0        digital, UCA0STE
    NotARegister,              //5           P1.1        digital, UCA0CLK
    NotARegister,              //6           P1.2        digital, UCA0RXD, UCA0SOMI
    NotARegister,              //7           P1.3        digital, UCA0TXD, UCA0SIMO
    NotARegister,              //8           P1.4        digital, UCB0STE
    NotARegister,              //9           P1.5        digital, UCB0CLK
    NotARegister,              //10          P1.6        digital, UCB0SIMO, UCB0SDA
    NotARegister,              //11          P1.7        digital, UCB0SOMI, UCB0SCL
    NotARegister,              //12          N/A         VCORE
    NotARegister,              //13          N/A         DVCC1
    NotARegister,              //14          N/A         VSW
    NotARegister,              //15          N/A         DVSS1
    NotARegister,              //16          P2.0        digital, PM_UCA1STE
    NotARegister,              //17          P2.1        digital, PM_UCA1CLK
    NotARegister,              //18          P2.2        digital, PM_UCA1RXD, PM_UCA1SOMI
    NotARegister,              //19          P2.3        digital, PM_UCA1TXD, PM_UCA1SIMO
    TIMER_A_CAPTURECOMPARE_REGISTER_1,              //20          P2.4        digital, PM_TA0.1
    TIMER_A_CAPTURECOMPARE_REGISTER_2,              //21          P2.5        digital, PM_TA0.2
    TIMER_A_CAPTURECOMPARE_REGISTER_3,              //22          P2.6        digital, PM_TA0.3
    TIMER_A_CAPTURECOMPARE_REGISTER_4,              //23          P2.7        digital, PM_TA0.4
    NotARegister,              //24          P10.4       digital, TA3.0, C0.7
    TIMER_A_CAPTURECOMPARE_REGISTER_1,              //25          P10.5       digital, TA3.1, C0.6
    TIMER_A_CAPTURECOMPARE_REGISTER_4,              //26          P7.4        digital, PM_TA1.4, C0.5
    TIMER_A_CAPTURECOMPARE_REGISTER_3,              //27          P7.5        digital, PM_TA1.3, C0.4
    TIMER_A_CAPTURECOMPARE_REGISTER_2,              //28          P7.6        digital, PM_TA1.2, C0.3
    TIMER_A_CAPTURECOMPARE_REGISTER_1,              //29          P7.7        digital, PM_TA1.1, C0.2
    NotARegister,              //30          P8.0        digital, UCB3STE, TA1.0, C0.1
    NotARegister,              //31          P8.1        digital, UCB3CLK, TA2.0, C0.0
    NotARegister,              //32          P3.0        digital, PM_UCA2STE
    NotARegister,              //33          P3.1        digital, PM_UCA2CLK
    NotARegister,              //34          P3.2        digital, PM_UCA2RXD, PM_UCA2SOMI
    NotARegister,              //35          P3.3        digital, PM_UCA2TXD, PM_UCA2SIMO
    NotARegister,              //36          P3.4        digital, PM_UCB2STE
    NotARegister,              //37          P3.5        digital, PM_UCB2CLK
    NotARegister,              //38          P3.6        digital, PM_UCB2SIMO, PM_UCB2SDA
    NotARegister,              //39          P3.7        digital, PM_UCB2SOMI, PM_UCB2SCL
    NotARegister,              //40          N/A         AVSS3
    NotARegister,              //41          PJ.0        digital, LFXIN  Technically can be used, but it's a crystal CLK port so way too important
    NotARegister,              //42          PJ.1        digital, LFXOUT
    NotARegister,              //43          N/A         AVSS1
    NotARegister,              //44          N/A         DCOR
    NotARegister,              //45          N/A         AVCC1
    TIMER_A_CAPTURECOMPARE_REGISTER_2,              //46          P8.2        digital, TA3.2, A23
    NotARegister,              //47          P8.3        digital, TA3CLK, A22
    NotARegister,              //48          P8.4        digital, A21
    NotARegister,              //49          P8.5        digital, A20
    NotARegister,              //50          P8.6        digital, A19
    NotARegister,              //51          P8.7        digital, A18
    NotARegister,              //52          P9.0        digital, A17
    NotARegister,              //53          P9.1        digital, A16
    NotARegister,              //54          P6.0        digital, A15
    NotARegister,              //55          P6.1        digital, A14
    NotARegister,              //56          P4.0        digital, A13
    NotARegister,              //57          P4.1        digital, A12
    NotARegister,              //58          P4.2        digital, ACLK, TA2CLK, A11
    NotARegister,              //59          P4.3        digital, MCLK, RTCCLK, A10
    NotARegister,              //60          P4.4        digital, HSMCLK, SVMHOUT, A9
    NotARegister,              //61          P4.5        digital, A8
    NotARegister,              //62          P4.6        digital, A7
    NotARegister,              //63          P4.7        digital, A6
    NotARegister,              //64          P5.0        digital, A5
    NotARegister,              //65          P5.1        digital, A4
    NotARegister,              //66          P5.2        digital, A3
    NotARegister,              //67          P5.3        digital, A2
    NotARegister,              //68          P5.4        digital, A1
    NotARegister,              //69          P5.5        digital, A0
    TIMER_A_CAPTURECOMPARE_REGISTER_1,              //70          P5.6        digital, TA2.1, VREF+, VeREF+, C1.7
    TIMER_A_CAPTURECOMPARE_REGISTER_2,              //71          P5.7        digital, TA2.2, VREF-, VeREF-, C1.6
    NotARegister,              //72          N/A         DVSS2
    NotARegister,              //73          N/A         DVCC2
    TIMER_A_CAPTURECOMPARE_REGISTER_3,              //74          P9.2        digital, TA3.3
    TIMER_A_CAPTURECOMPARE_REGISTER_4,              //75          P9.3        digital, TA3.4
    NotARegister,              //76          P6.2        digital, UCB1STE, C1.5
    NotARegister,              //77          P6.3        digital, UCB1CLK, C1.4
    NotARegister,              //78          P6.4        digital, UCB1SIMO, UCB1SDA, C1.3
    NotARegister,              //79          P6.5        digital, UCB1SOMI, UCB1SCL, C1.2
    TIMER_A_CAPTURECOMPARE_REGISTER_3,              //80          P6.6        digital, TA2.3, UCB3SIMO, UCB3SDA, C1.1
    TIMER_A_CAPTURECOMPARE_REGISTER_4,              //81          P6.7        digital, TA2.4, UCB3SOMI, UCB3SCL, C1.0
    NotARegister,              //82          N/A         DVSS3
    NotARegister,              //83          N/A         !reset, NMI
    NotARegister,              //84          N/A         AVSS2
    NotARegister,              //85          PJ.2        digital, HFXOUT
    NotARegister,              //86          PJ.3        digital, HFXIN
    NotARegister,              //87          N/A         AVCC2
    NotARegister,              //88          P7.0        digital, PM_SMCLK, PM_DMAE0
    NotARegister,              //89          P7.1        digital, PM_C0OUT, PM_TA0CLK
    NotARegister,              //90          P7.2        digital, PM_C1OUT, PM_TA1CLK
    NotARegister,              //91          P7.3        digital, PM_TA0.0
    NotARegister,              //92          PJ.4        digital, TDI
    NotARegister,              //93          PJ.5        digital, TD0, SWO
    NotARegister,              //94          N/A         SWDIOTMS
    NotARegister,              //95          N/A         SWCLKTCK
    NotARegister,              //96          P9.4        digital, UCA3STE
    NotARegister,              //97          P9.5        digital, UCA3CLK
    NotARegister,              //98          P9.6        digital, UCA3RXD, UCA3SOMI
    NotARegister,              //99          P9.7        digital, UCA3TXD, UCA3SIMO
    NotARegister               //100         P10.0       digital, UCB3STE
};

//retreives generator related to the specific mapped pin
static uint32_t pinToTimerA[] =
{
    NotARegister,              //dummy
    NotARegister,              //1           P10.1       digital, UCB3CLK
    NotARegister,              //2           P10.2       digital, UCB3SIMO, UCB3SDA
    NotARegister,              //3           P10.3       digital, UCB3SOMI, UCB3SCL
    NotARegister,              //4           P1.0        digital, UCA0STE
    NotARegister,              //5           P1.1        digital, UCA0CLK
    NotARegister,              //6           P1.2        digital, UCA0RXD, UCA0SOMI
    NotARegister,              //7           P1.3        digital, UCA0TXD, UCA0SIMO
    NotARegister,              //8           P1.4        digital, UCB0STE
    NotARegister,              //9           P1.5        digital, UCB0CLK
    NotARegister,              //10          P1.6        digital, UCB0SIMO, UCB0SDA
    NotARegister,              //11          P1.7        digital, UCB0SOMI, UCB0SCL
    NotARegister,              //12          N/A         VCORE
    NotARegister,              //13          N/A         DVCC1
    NotARegister,              //14          N/A         VSW
    NotARegister,              //15          N/A         DVSS1
    NotARegister,              //16          P2.0        digital, PM_UCA1STE
    NotARegister,              //17          P2.1        digital, PM_UCA1CLK
    NotARegister,              //18          P2.2        digital, PM_UCA1RXD, PM_UCA1SOMI
    NotARegister,              //19          P2.3        digital, PM_UCA1TXD, PM_UCA1SIMO
    TIMER_A0_BASE,             //20          P2.4        digital, PM_TA0.1
    TIMER_A0_BASE,             //21          P2.5        digital, PM_TA0.2
    TIMER_A0_BASE,             //22          P2.6        digital, PM_TA0.3
    TIMER_A0_BASE,             //23          P2.7        digital, PM_TA0.4
    NotARegister,              //24          P10.4       digital, TA3.0, C0.7
    TIMER_A3_BASE,             //25          P10.5       digital, TA3.1, C0.6
    TIMER_A1_BASE,             //26          P7.4        digital, PM_TA1.4, C0.5
    TIMER_A1_BASE,             //27          P7.5        digital, PM_TA1.3, C0.4
    TIMER_A1_BASE,             //28          P7.6        digital, PM_TA1.2, C0.3
    TIMER_A1_BASE,             //29          P7.7        digital, PM_TA1.1, C0.2
    NotARegister,              //30          P8.0        digital, UCB3STE, TA1.0, C0.1
    NotARegister,              //31          P8.1        digital, UCB3CLK, TA2.0, C0.0
    NotARegister,              //32          P3.0        digital, PM_UCA2STE
    NotARegister,              //33          P3.1        digital, PM_UCA2CLK
    NotARegister,              //34          P3.2        digital, PM_UCA2RXD, PM_UCA2SOMI
    NotARegister,              //35          P3.3        digital, PM_UCA2TXD, PM_UCA2SIMO
    NotARegister,              //36          P3.4        digital, PM_UCB2STE
    NotARegister,              //37          P3.5        digital, PM_UCB2CLK
    NotARegister,              //38          P3.6        digital, PM_UCB2SIMO, PM_UCB2SDA
    NotARegister,              //39          P3.7        digital, PM_UCB2SOMI, PM_UCB2SCL
    NotARegister,              //40          N/A         AVSS3
    NotARegister,              //41          PJ.0        digital, LFXIN  Technically can be used, but it's a crystal CLK port so way too important
    NotARegister,              //42          PJ.1        digital, LFXOUT
    NotARegister,              //43          N/A         AVSS1
    NotARegister,              //44          N/A         DCOR
    NotARegister,              //45          N/A         AVCC1
    TIMER_A3_BASE,             //46          P8.2        digital, TA3.2, A23
    NotARegister,              //47          P8.3        digital, TA3CLK, A22
    NotARegister,              //48          P8.4        digital, A21
    NotARegister,              //49          P8.5        digital, A20
    NotARegister,              //50          P8.6        digital, A19
    NotARegister,              //51          P8.7        digital, A18
    NotARegister,              //52          P9.0        digital, A17
    NotARegister,              //53          P9.1        digital, A16
    NotARegister,              //54          P6.0        digital, A15
    NotARegister,              //55          P6.1        digital, A14
    NotARegister,              //56          P4.0        digital, A13
    NotARegister,              //57          P4.1        digital, A12
    NotARegister,              //58          P4.2        digital, ACLK, TA2CLK, A11
    NotARegister,              //59          P4.3        digital, MCLK, RTCCLK, A10
    NotARegister,              //60          P4.4        digital, HSMCLK, SVMHOUT, A9
    NotARegister,              //61          P4.5        digital, A8
    NotARegister,              //62          P4.6        digital, A7
    NotARegister,              //63          P4.7        digital, A6
    NotARegister,              //64          P5.0        digital, A5
    NotARegister,              //65          P5.1        digital, A4
    NotARegister,              //66          P5.2        digital, A3
    NotARegister,              //67          P5.3        digital, A2
    NotARegister,              //68          P5.4        digital, A1
    NotARegister,              //69          P5.5        digital, A0
    TIMER_A2_BASE,             //70          P5.6        digital, TA2.1, VREF+, VeREF+, C1.7
    TIMER_A2_BASE,             //71          P5.7        digital, TA2.2, VREF-, VeREF-, C1.6
    NotARegister,              //72          N/A         DVSS2
    NotARegister,              //73          N/A         DVCC2
    TIMER_A3_BASE,             //74          P9.2        digital, TA3.3
    TIMER_A3_BASE,             //75          P9.3        digital, TA3.4
    NotARegister,              //76          P6.2        digital, UCB1STE, C1.5
    NotARegister,              //77          P6.3        digital, UCB1CLK, C1.4
    NotARegister,              //78          P6.4        digital, UCB1SIMO, UCB1SDA, C1.3
    NotARegister,              //79          P6.5        digital, UCB1SOMI, UCB1SCL, C1.2
    TIMER_A2_BASE,             //80          P6.6        digital, TA2.3, UCB3SIMO, UCB3SDA, C1.1
    TIMER_A2_BASE,             //81          P6.7        digital, TA2.4, UCB3SOMI, UCB3SCL, C1.0
    NotARegister,              //82          N/A         DVSS3
    NotARegister,              //83          N/A         !reset, NMI
    NotARegister,              //84          N/A         AVSS2
    NotARegister,              //85          PJ.2        digital, HFXOUT
    NotARegister,              //86          PJ.3        digital, HFXIN
    NotARegister,              //87          N/A         AVCC2
    NotARegister,              //88          P7.0        digital, PM_SMCLK, PM_DMAE0
    NotARegister,              //89          P7.1        digital, PM_C0OUT, PM_TA0CLK
    NotARegister,              //90          P7.2        digital, PM_C1OUT, PM_TA1CLK
    NotARegister,              //91          P7.3        digital, PM_TA0.0
    NotARegister,              //92          PJ.4        digital, TDI
    NotARegister,              //93          PJ.5        digital, TD0, SWO
    NotARegister,              //94          N/A         SWDIOTMS
    NotARegister,              //95          N/A         SWCLKTCK
    NotARegister,              //96          P9.4        digital, UCA3STE
    NotARegister,              //97          P9.5        digital, UCA3CLK
    NotARegister,              //98          P9.6        digital, UCA3RXD, UCA3SOMI
    NotARegister,              //99          P9.7        digital, UCA3TXD, UCA3SIMO
    NotARegister               //100         P10.0       digital, UCB3STE
};

static TimerAPwmConfig* getPwmModule(uint16_t moduleIndex)
{
  TimerAPwmConfig* module = 0;
  switch(moduleIndex)
  {
    case 0:
      module = &pwm0Config;
      break;

    case 1:
      module = &pwm1Config;
      break;

    case 2:
      module = &pwm2Config;
      break;

    case 3:
      module = &pwm3Config;
      break;
  }

  return module;
}

bool validateInput(uint8_t writeModule, uint8_t pin)
{
  switch(writeModule)
  {
    case 0:
      if(pin == P2_4 || pin == P2_5 || pin == P2_6 || pin == P2_7)
      {
        return true;
      }
      break;

    case 1:
      if(pin == P7_7 || pin == P7_6 || pin == P7_5 || pin == P7_4)
      {
        return true;
      }
      break;

    case 2:
      if(pin == P5_6 || pin == P5_7 || pin == P6_6 || pin == P6_7)
      {
        return true;
      }
      break;

    case 3:
      if(pin == P10_5 || pin == P8_2 || pin == P9_2 || pin == P9_3)
      {
        return true;
      }
      break;
  }

  return false;
}

rovePwmWrite_Handle setupPwmWrite(uint8_t writeModule, uint8_t pin)
{
  rovePwmWrite_Handle handle;
  TimerAPwmConfig* moduleData = getPwmModule(writeModule);

  if(pin > 100)
  {
    debugFault("setupPwmWrite: pin value is nonsense");
  }
  else if (writeModule > 3)
  {
    debugFault("setupPwmWrite: writeModule value is nonsense");
  }
  else if(validateInput(writeModule, pin) == false)
  {
    debugFault("setupPwmWrite: module doesn't fit pin");
  }

  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(pinToPinPort[pin], pinToPinMask[pin], GPIO_PRIMARY_MODULE_FUNCTION);

  handle.initialized = true;
  handle.index = writeModule;
  handle.pin = pin;
  handle.alignment = LeftAligned;
  handle.inverted = false;
  handle.duty = 0;

  setPwmTotalPeriod(handle, DefaultPeriodUs);
  return handle;
}

void pwmWriteDuty(rovePwmWrite_Handle handle, uint8_t duty)
{
  float percentDuty;
  TimerAPwmConfig* moduleData;
  Timer_A_PWMConfig config;

  if(handle.initialized == false)
  {
    debugFault("pwmWriteDuty: handle not initialized");
  }
  else if(duty == 0)
  {
    GPIO_setAsOutputPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    GPIO_setOutputLowOnPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    handle.duty = 0;
  }
  else if(duty == 255)
  {
    GPIO_setAsOutputPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    GPIO_setOutputHighOnPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    handle.duty = 255;
  }
  else
  {
    moduleData = getPwmModule(handle.index);
    handle.duty = duty;
    percentDuty = (float)duty/255.0; //0-255 is input for duty, being 8 bit

    config.clockSource = moduleData->clockSource;
    config.clockSourceDivider = moduleData->clockSourceDivider;
    config.timerPeriod = moduleData->timerPeriod;
    config.compareOutputMode = alignmentToOutputMode[handle.alignment];
    config.compareRegister = pinToCCRegister[handle.pin];
    config.dutyCycle =  ((float)moduleData->timerPeriod) * percentDuty;

    Timer_A_generatePWM(pinToTimerA[handle.pin], &config);
  }
}

void pwmWriteWidth(rovePwmWrite_Handle handle, uint32_t pulseW_us)
{
  TimerAPwmConfig* moduleData = getPwmModule(handle.index);
  Timer_A_PWMConfig config;

  if(handle.initialized == false)
  {
    debugFault("pwmWriteDuty: handle not initialized");
  }
  else if(pulseW_us == 0)
  {
    GPIO_setAsOutputPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    GPIO_setOutputLowOnPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    handle.duty = 0;
  }
  else if(pulseW_us >= moduleData->getTimerPeriodUs())
  {
    GPIO_setAsOutputPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    GPIO_setOutputHighOnPin(pinToPinPort[handle.pin], pinToPinMask[handle.pin]);
    handle.duty = 255;
  }
  else
  {
    handle.duty = (pulseW_us * 100) / moduleData->getTimerPeriodUs();
    handle.duty = map(handle.duty, 0, 100, 0, 255);

    config.clockSource = moduleData->clockSource;
    config.clockSourceDivider = moduleData->clockSourceDivider;
    config.timerPeriod = moduleData->timerPeriod;
    config.compareOutputMode = alignmentToOutputMode[handle.alignment];
    config.compareRegister = pinToCCRegister[handle.pin];
    config.dutyCycle = moduleData->pulseWidthUsToTimerTicks(pulseW_us);

    Timer_A_generatePWM(pinToTimerA[handle.pin], &config);
  }
}

void setPwmTotalPeriod(rovePwmWrite_Handle handle, uint32_t pulsePeriod_us)
{
  uint32_t clockSource;
  uint8_t clockDivisor;
  TimerAPwmConfig * moduleData;

  if(handle.initialized == false)
  {
    debugFault("setPwmPeriod: handle not initialized");
  }

  moduleData = getPwmModule(handle.index);
  if(getOptimalClockSettings(pulsePeriod_us, moduleData, &clockSource, &clockDivisor) == 0)
  {
    return;
  }

  moduleData->clockSource = clockSource;
  moduleData->clockSourceDivider = clockDivisor;
  moduleData->timerPeriod = moduleData->timerPeriodUsToTimerPeriodTicks(pulsePeriod_us);

  //if we're currently writing out a pwm waveform, re-set it up and continue writing
  if(0 < handle.duty && handle.duty < 255)
  {
    pwmWriteDuty(handle, handle.duty);
  }
}

void setPwmAlignment(rovePwmWrite_Handle handle, pwmAlignment alignment)
{
  if(handle.initialized == false)
  {
   debugFault("setPwmAlignment: handle not initialized");
  }

  handle.alignment = alignment;

  //if we're currently writing out a pwm waveform, re-set it up and continue writing
  if(0 < handle.duty && handle.duty < 255)
  {
    pwmWriteDuty(handle, handle.duty);
  }
}

//configures the pwm clock source for operation by selecting the most accurate clock that can service the desired maximum pulse period.
//returns true if settings were found, false if no setting worked
static bool getOptimalClockSettings(uint32_t pulsePeriod_us, TimerAPwmConfig *currentConfig, uint32_t *clockSource, uint8_t *clockDivisor)
{
  uint8_t divisorFound;
  uint32_t sourceFound;
  uint32_t loadWithCurrentSettings = getTimerPeriodTicks(pulsePeriod_us, currentConfig->getClockSourceFreq(), currentConfig->clockSourceDivider);

  //in general, the system is more accurate the faster the clocks can go. But the faster they go, the smaller their maximum period is.
  //If the current clock source and divisor is too fast, we have to try to slow it down until we fit the max
  if(loadWithCurrentSettings > PeriodMaxLoad)
  {
    //search fastest first. Then if that clock source didn't work, try the other one
    if(CS_getACLK() > CS_getSMCLK())
    {
      sourceFound = TIMER_A_CLOCKSOURCE_ACLK;
      divisorFound = getOptimalDivisorGoingSlower(pulsePeriod_us, CS_getACLK());

      if(divisorFound == 0)
      {
        sourceFound = TIMER_A_CLOCKSOURCE_SMCLK;
        divisorFound = getOptimalDivisorGoingSlower(pulsePeriod_us, CS_getSMCLK());

        if(divisorFound == 0)
        {
          return false;
        }
      }
    }
  }

  //current settings can work, but let's choose the most accurate of the bunch
  else if(loadWithCurrentSettings < PeriodMaxLoad)
  {
    //search fastest first. Then if that clock source didn't work, try the other one
    if(CS_getACLK() > CS_getSMCLK())
    {
     sourceFound = TIMER_A_CLOCKSOURCE_ACLK;
     divisorFound = getOptimalDivisorGoingFaster(pulsePeriod_us, CS_getACLK());

     if(divisorFound == 0)
     {
       sourceFound = TIMER_A_CLOCKSOURCE_SMCLK;
       divisorFound = getOptimalDivisorGoingFaster(pulsePeriod_us, CS_getSMCLK());

       if(divisorFound == 0)
       {
         return false;
       }
     }
    }
  }

  //what do you know, this is already the best we can get
  else
  {
    divisorFound = *clockDivisor;
    sourceFound = *clockSource;
  }

  *clockDivisor = divisorFound;
  *clockSource = sourceFound;

  return true;
}

static uint32_t getTimerPeriodTicks(const uint32_t pulsePeriod_us, const uint32_t clockFreq, const uint8_t clockDivisor)
{
  return (pulsePeriod_us * 1000000) / (clockFreq / clockDivisor);
}

static uint32_t getOptimalDivisorGoingSlower(const uint32_t pulsePeriod_us, const uint32_t clockFreq)
{
  return getOptimalDivisorGoingSlower(pulsePeriod_us, clockFreq, 1); //start with the fastest divisor, 1, and go slower from there
}

static uint32_t getOptimalDivisorGoingSlower(const uint32_t pulsePeriod_us, const uint32_t clockFreq, uint8_t clockDivisor)
{
  uint32_t thisPeriod = getTimerPeriodTicks(pulsePeriod_us, clockFreq, clockDivisor);
  if(thisPeriod <= PeriodMaxLoad)
  {
    return clockDivisor;
  }
  else
  {
    //valid values are weird, not simply multiplied by two every time
    if(clockDivisor < 8)
    {
      clockDivisor += 1;
    }
    else if(clockDivisor < 16)
    {
      clockDivisor += 2;
    }
    else if(clockDivisor < 32)
    {
      clockDivisor += 4;
    }
    else if(clockDivisor < 64)
    {
      clockDivisor += 8;
    }
    else
    {
      return 0; //no divider worked for this clock
    }

    return getOptimalDivisorGoingSlower(pulsePeriod_us, clockFreq, clockDivisor);
  }
}

static uint32_t getOptimalDivisorGoingFaster(const uint32_t pulsePeriod_us, const uint32_t clockFreq)
{
  return getOptimalDivisorGoingFaster(pulsePeriod_us, clockFreq, 64); //start with the slowest divisor of 64 and work up
}

static uint32_t getOptimalDivisorGoingFaster(const uint32_t pulsePeriod_us, const uint32_t clockFreq, const uint8_t clockDivisor)
{
  const int UseLastCalculation = 255;
  uint8_t newDivisor = clockDivisor;
  uint8_t retVal;
  uint32_t thisPeriod = getTimerPeriodTicks(pulsePeriod_us, clockFreq, clockDivisor);

  //if we're already at 64 and above the max period, then this clock in general is too fast
  if(thisPeriod > PeriodMaxLoad && clockDivisor >= 64)
  {
    return 0;
  }

  //if we just now went above the max, then it means the last calculated divisor was the closest we could get
  else if(thisPeriod > PeriodMaxLoad)
  {
    return UseLastCalculation;
  }

  //this period is under the max. Keep going till we cross the max
  else
  {
    //valid values are weird, not simply multiplied by two every time
    if(clockDivisor == 1)
    {
      return 1; //best we can get on this clock, we never hit our limit
    }
    else if(clockDivisor <= 8)
    {
      newDivisor = clockDivisor - 1;
    }
    else if(clockDivisor <= 16)
    {
      newDivisor = clockDivisor - 2;
    }
    else if(clockDivisor <= 32)
    {
      newDivisor = clockDivisor - 4;
    }
    else if(clockDivisor <= 64)
    {
      newDivisor = clockDivisor - 8;
    }

    retVal =  getOptimalDivisorGoingFaster(pulsePeriod_us, clockFreq, newDivisor);

    //if the next loop says that it hit the max, then it means the clock divisor in this loop
    //is the best available
    if(retVal == UseLastCalculation)
    {
      return clockDivisor;
    }
    else
    {
      return retVal;
    }
  }
}
