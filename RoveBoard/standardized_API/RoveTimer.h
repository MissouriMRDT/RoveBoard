#ifndef TIMERINTERFACE_H_
#define TIMERINTERFACE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct roveTimer_Handle
{
    bool initialized;
    uint16_t index;

    roveTimer_Handle()
    {
      initialized = false;
    }
} roveTimer_Handle;

//interrupt id's
#define TimerPeriodicInterrupt 0

//sets up the specified timer to generate the specified interrupt at a specified rate
//Input: Timer Id (hardware dependant), the interruptId based on above constants, how frequently
//the timer runs the interrupt in microseconds, and what function to run everytime it interrupts
roveTimer_Handle setupTimer(uint32_t timerId, uint32_t interruptId, uint32_t timerTimeout_us, void (*interruptFunc)(void));

//begins timer operation
//inputs: handle of the timer to start
void startTimer(roveTimer_Handle handle);

//stops timer operation
//inputs: handle of the timer to stop
void stopTimer(roveTimer_Handle handle);

#endif
