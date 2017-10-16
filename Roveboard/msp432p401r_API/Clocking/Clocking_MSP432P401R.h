#ifndef CLOCKING_TIVATM4C1294NCPDT_H_
#define CLOCKING_TIVATM4C1294NCPDT_H_

/* Programmer: Drue Satterfield
 * Date of creation: September 2017
 * Microcontroller used: MSP432P401R
 * Hardware components used by this file: Hardware CPU clock, and system tick timer.
 *
 * Description: This library is used to allow the user to control the main CPU clock, as well as control other functions
 * relating to clocking and timing such as delaying and seeing how many microseconds have passed since program began.
 * The default state when initialization is called is for the CPU to go as fast as it can, 48Mhz.
 *
 * Warnings:
 * The delaying and millis/micros functions all rely on the system tick timer;
 * do not modify the system tick timer unless you absolutely have to, and make sure you know what the effects will be.
 *
 * The functions that modify the main system clock should be done at the beginning of the program, and should be relatively constant
 * thereafter; some hardware components when initialized will stupidly keep thinking the clock is the same value that it was upon its initialization.
 * If changes to the CPU clock is made, most other libraries will need to be re-initialized.
 */

#include <stdbool.h>
#include <stdint.h>

#define EXTERNAL_LOW_FREQUENCY_CRYSTAL_FREQ 32000 //standard msp432 boards use 32khz crystals for low frequency, and 48Mhz for high frequency
#define EXTERNAL_HIGH_FREQUENCY_CRYSTAL_FREQ 48000000

// the msp432 has two voltage modes, high and min voltage, based on how fast the clocks are going. This is the highest frequency that
//the main CPU clock can be going while still in min voltage mode. Beyond this the software will change to high voltage mode.
#define MinVoltageMode_CpuMaxFreq 24000000
#define MinVoltageMode_PeriphMaxFreq 12000000

void initSystemClocks();

uint32_t setCpuClockFreq(uint32_t newFrequency);
uint32_t setPeriphClockFreq(uint32_t newFrequency);

//warning: this might affect the current speeds of the periph and cpu clocks. It's recommended that you call
//setCpuClockFreq and setPeriphClockFreq again after modifying the programmable clock source
uint32_t setProgrammableClockSourceFreq(uint32_t newFrequency);
uint32_t getCpuClockFreq();
uint32_t getPeriphClockFreq();

#ifdef __cplusplus
extern "C" {
#endif

void delay(uint32_t millis);
void delayMicroseconds(uint32_t microsToDelay);
uint32_t millis();
uint32_t micros();

#ifdef __cplusplus
}
#endif

#endif
