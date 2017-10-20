/*
 * Power_MSP432P401R.h
 *
 *  Created on: Oct 16, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_MSP432P401R_API_POWER_POWER_MSP432P401R_H_
#define ROVEBOARD_MSP432P401R_API_POWER_POWER_MSP432P401R_H_


bool useSwitchingPowerSource();
bool useLDOPowerSource();

void forceLowFrequencyMode(bool putInHighVoltageMode);
void forceLowVoltageMode();
void forceHighVoltageMode();


#endif /* ROVEBOARD_MSP432P401R_API_POWER_POWER_MSP432P401R_H_ */
