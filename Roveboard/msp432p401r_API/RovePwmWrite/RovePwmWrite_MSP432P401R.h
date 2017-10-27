/* Programmer: Drue Satterfield
 * Date of creation: 10/26/17
 * Microcontroller used: MSP432P401R
 *
 * This library is used to generate and output pwm waveforms. Internally, the system uses timers called TimerA's to do the generation,
 * and shares this hardware with any pwmReading capabilities or any other capture/compare operation.
 *
 * There are 4 TimerA's to output from, and each one has up to 4 outputs that can come from it. The outputs that come from a single TimerA
 * can have different duty cycles and pwm alignments, but will share the same wave period.
 *
 * The available pins and timerA's that use them are:
 *
 * TimerA_0      P2_4, P2_5, P2_6, P2_7
 * TimerA_1      P7_7, P7_6, P7_5, P7_4
 * TimerA_2      P5_6, P5_7, P6_6, P6_7
 * TimerA_3      P8_2, P9_2, P9_3, P10_5
 *
 * (For those who have read the datasheet: Yes there's technically another pin they can each output from, but that register determines the overall
 *  period of the waveform so shouldn't be messed with)
 *
 * Warning: Timing notices:
 * The maximum waveform period depends on the current speed of the system clocks. If the HSMCLK is less than 128khz (by default it's 24Mhz) then
 * the maximum waveform period is 37 seconds. Otherwise, the max is 2^16 / (HSMCLK / 64).
 * The minimum waveform period, if the HSMCLK is less than 128khz, is 8 microseconds. Else it's 1 / HSMCLK.
 * In general, the faster the clock, the smaller the maximum waveform period, and the greater the accuracy. The best thing is to usually
 * select the fastest clock speed that can still give you the waveform period you want. Internally, the system will attempt to find the optimal
 * solution everytime you change the period, by cycling through available clock sources and dividing them down as much as the chip will allow them
 * to be divided (up to a divisor of 64) until it finds the best solution.
 *
 * Warning: When the system clock speed changes:
 * When changes to the system clock speed is done, it's best to call the init function again so that it can correct for changes in the system
 * clock speed
 */

#ifndef PWMWRITER_TIVATM4C1294NCPDT_H_
#define PWMWRITER_TIVATM4C1294NCPDT_H_

#include <stdint.h>
#include <stdbool.h>
#include "../../standardized_API/RovePwmWrite.h"

//Decides the alignment of the PWM wave. That is, where the pulse is generated in the overall waveform.
//For example, leftAligned generates a pulse like this:   --____
//right aligned would look like this: ____--
typedef enum pwmAlignment {LeftAligned = 0, RightAligned = 1} pwmAlignment;

const int TimerA0 = 0;
const int TimerA1 = 1;
const int TimerA2 = 2;
const int TimerA3 = 3;

//sets up the pwm generator to be able to write a pwm wave on the specified pin
//inputs:
//  writeModule: the timer to use to write a pwm wave, 0 to 3
//  pin: the pin to output the pwm wave on, must is used by the module. See header comments above for which pins are used by which generator.
//returns: a handle to the initialized pwm instance
//warning: Function will enter an infinite fault loop if arguments are incorrect
//note: You CAN call this multiple times to set up a module with different pins without issue.
//note: Default settings are: Left aligned wave, 2041 microsecond total period
rovePwmWrite_Handle setupPwmWrite(uint8_t writeModule, uint8_t pin);

//outputs a pwm wave at the specified duty cycle
//inputs:
//  handle: the handle of the pwm instance to write with
//  duty: duty cycle, between 0 and 255 with 0 being 0% and 255 being 100% duty cycle
void pwmWriteDuty(rovePwmWrite_Handle handle, uint8_t duty);

//outputs a pwm wave at the specified pulse width
//inputs:
//  handle: the handle of the pwm instance to write with
//  pulseW_us: width of the pwm's On period, in microseconds. 0 for 0% duty cycle, and >= to the pulse total period for 100% duty cycle.
void pwmWriteWidth(rovePwmWrite_Handle handle, uint32_t pulseW_us);

//sets the pwm total period for this generator.
//inputs:
//  handle: the handle of the pwm instance to write with
//  pulsePeriod_us: width of the entire pwm wave, in microseconds. Should never exceed 32 bits(3 min or so, quite the long pwm wave)
//warning: This will affect all the handles that are using the timer this handle uses.
void setPwmTotalPeriod(rovePwmWrite_Handle handle, uint32_t pulsePeriod_us);

//sets the pwm wave alignment for this generator.
//inputs:
//  handle: the handle of the pwm instance to write with
//  alignment: pwm wave alignment
//warning: This will affect all the handles that are using the timer this handle uses
void setPwmAlignment(rovePwmWrite_Handle handle, pwmAlignment alignment);

#endif /* PWMWRITER_H_ */

