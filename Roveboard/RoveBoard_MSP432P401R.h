/*
 * RoveBoard_MSP432P401R.h
 *
 *  Created on: Oct 15, 2017
 *      Author: drue
 *
 *  Primary external include for main.cpp and similar, non-roveware files, whenever the board being used is the
 *  MSP432P401R
 */

#ifndef ROVEBOARD_ROVEBOARD_MSP432P401R_H_
#define ROVEBOARD_ROVEBOARD_MSP432P401R_H_

#ifndef __MSP432P401R__
#define __MSP432P401R__
#endif

#include "supportingUtilities/RoveUtilities.h"
#include "supportingUtilities/Debug.h"
#include "msp432p401r_API/Clocking/Clocking_MSP432P401R.h"
#include "msp432p401r_API/SysInit/SysInit_MSP432P401R.h"

static int dummySoThisRunsBeforeMain = (initSystem(), 0);

#endif /* ROVEBOARD_ROVEBOARD_MSP432P401R_H_ */
