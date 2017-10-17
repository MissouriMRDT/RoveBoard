/*
 * Power_MSP432P401R.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: drue
 */

#include "Power_MSP432P401R.h"
#include "../Clocking/Clocking_MSP432P401R.h"
#include "../msp432Ware/driverlib/driverlib.h"

#define MinVoltageMode_CpuMaxFreq 24000000
#define MinVoltageMode_PeriphMaxFreq 12000000
#define MaxVoltageMode_CpuMaxFreq 48000000
#define MaxVoltageMode_PeriphMaxFreq 24000000
#define LowFreqMode_Freq 128000

bool useSwitchingPowerSource()
{
  return PCM_setPowerMode(PCM_DCDC_MODE);
}

bool useLDOPowerSource()
{
  return PCM_setPowerMode(PCM_LDO_MODE);
}

void forceLowFrequencyMode()
{
  setCpuClockFreq(LowFreqMode_Freq);
  setPeriphClockFreq(LowFreqMode_Freq);
  PCM_setPowerMode(PCM_LF_MODE);
}

void forceLowVoltageMode()
{
  //setClockFreq functions automatically select the most optimal voltage state, so setting both frequencies to their
  //lowest needed for min voltage mode will cause it to enter min voltage mode
  setCpuClockFreq(MinVoltageMode_CpuMaxFreq);
  setPeriphClockFreq(MinVoltageMode_PeriphMaxFreq);
}

void forceHighVoltageMode()
{
  setCpuClockFreq(MaxVoltageMode_CpuMaxFreq);
  setPeriphClockFreq(MaxVoltageMode_PeriphMaxFreq);
}
