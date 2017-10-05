/*
 * RoveBoard.h
 *
 *  Created on: Sep 4, 2017
 *      Author: drue
 *
 *  Primary external include for roveware files like RoveComm. Main.cpp and related files shouldn't
 *  call this, on the other hand, but instead call at the program beginning the version of RoveBoard
 *  associated with the chip the program is being ran on.
 */

#ifndef ROVEBOARD_ROVEBOARD_H_
#define ROVEBOARD_ROVEBOARD_H_

#include "generics/Clocking.h"
#include "generics/DigitalPin.h"
#include "generics/RoveEthernet.h"
#include "generics/RoveI2C.h"
#include "generics/RovePwmRead.h"
#include "generics/RovePwmWrite.h"
#include "generics/RoveTimer.h"
#include "generics/RoveUart.h"
#include "utilities/RoveUtilities.h"


#endif /* ROVEBOARD_ROVEBOARD_H_ */
