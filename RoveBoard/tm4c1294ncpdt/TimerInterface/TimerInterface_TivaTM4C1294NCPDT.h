#ifndef TIMERINTERFACE_TIVATM4C1294NCPDT_H_
#define TIMERINTERFACE_TIVATM4C1294NCPDT_H_

/* Programmer:  Drue Satterfield
 * Date of creation: September 2017.
 * Microcontroller used: Tiva TM4C1294NCPDT
 * Hardware components used by this file: all timers can possibly be used, from 0 to 7; timers are explicitely called for operation by user
 *    and when not initialized can be used for other projects instead.
 *
 *
 * Description: This library is used to implement basic timer capabilities relating to timing out and periodic interrupting.
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>

//timer id's
const uint8_t Timer0 = 0;
const uint8_t Timer1 = 1;
const uint8_t Timer2 = 2;
const uint8_t Timer3 = 3;
const uint8_t Timer4 = 4;
const uint8_t Timer5 = 5;
const uint8_t Timer6 = 6;
const uint8_t Timer7 = 7;

//interrupt id's
#define TimerPeriodicInterrupt 0

//sets up the specified timer to generate the specified interrupt at a specified rate
//Input: Timer Id, the interruptId based on above constants, and how frequently
//the timer runs the interrupt in microseconds.
//Min is 1 us, max is about 268 seconds
void setupTimer(uint32_t timerId, uint32_t interruptId, uint32_t timerTimeout_us);

//begins timer operation
//inputs: timer Id
void startTimer(uint32_t timerId);

//stops timer operation
//inputs: timer Id
void stopTimer(uint32_t timerId);

//attaches a functions for the timer to run everytime it interrupts
//inputs: timer Id, an interrupt function to run
void attachTimerInterrupt(uint32_t timerId, void (*interruptFunc)(void) );

#endif
