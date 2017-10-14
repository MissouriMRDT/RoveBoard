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

#include "standardAPI_Prototypes/Clocking.h"
#include "standardAPI_Prototypes/DigitalPin.h"
#include "standardAPI_Prototypes/RoveEthernet.h"
#include "standardAPI_Prototypes/RoveI2C.h"
#include "standardAPI_Prototypes/RovePwmRead.h"
#include "standardAPI_Prototypes/RovePwmWrite.h"
#include "standardAPI_Prototypes/RoveTimer.h"
#include "standardAPI_Prototypes/RoveUart.h"
#include "supportingUtilities/RoveUtilities.h"
#include "supportingUtilities/Debug.h"


#endif /* ROVEBOARD_ROVEBOARD_H_ */
