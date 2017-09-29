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
#include "generics/PwmReader.h"
#include "generics/PwmWriter.h"
#include "generics/RoveEthernet.h"
#include "generics/RoveI2C.h"
#include "generics/RoveUart.h"
#include "generics/TimerInterface.h"
#include "utilities/RoveUtilities.h"


#endif /* ROVEBOARD_ROVEBOARD_H_ */
