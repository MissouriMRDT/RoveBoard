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

#include "prototype_API/Clocking.h"
#include "prototype_API/DigitalPin.h"
#include "prototype_API/RoveEthernet.h"
#include "prototype_API/RoveI2C.h"
#include "prototype_API/RovePwmRead.h"
#include "prototype_API/RovePwmWrite.h"
#include "prototype_API/RoveTimer.h"
#include "prototype_API/RoveUart.h"
#include "supportingUtilities/RoveUtilities.h"
#include "supportingUtilities/Debug.h"


#endif /* ROVEBOARD_ROVEBOARD_H_ */
