#ifndef ROVEBOARD_TIVATM4C1294NCPDT_H_
#define ROVEBOARD_TIVATM4C1294NCPDT_H_

/*
 * RoveBoard_TivaTM4C1294NCPDT.h
 *
 *  Created on: Sep 4, 2017
 *      Author: drue
 *
 *  Primary external include for main.cpp and similar, non-roveware files, whenever the board being used is the
 *  tiva tm4c1294ncpdt
 */

#ifndef PART_TM4C1294NCPDT
#define PART_TM4C1294NCPDT
#endif

#include "utilities/RoveUtilities.h"
#include "utilities/Debug.h"
#include "tm4c1294ncpdt/Clocking/Clocking_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/DigitalPin/DigitalPin_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RovePwmRead/RovePwmRead_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RovePwmWrite/RovePwmWrite_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RoveEthernet/RoveEthernet_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RovePinMap_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/SysInit/SysInit_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RoveTimer/RoveTimer_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RoveUart/RoveUart_TivaTM4C1294NCPDT.h"
#include "tm4c1294ncpdt/RoveI2C/RoveI2C_TivaTM4C1294NCPDT.h"

static int dummySoThisRunsBeforeMain = (initSystem(), 0);

#endif
