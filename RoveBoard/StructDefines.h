/*
 * StructDefines.h
 *
 *  Created on: Apr 18, 2018
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_

#if defined(PART_TM4C129ENCPDT) || defined(PART_TM4C1294NCPDT)

#include "tm4c1294ncpdt_API/RoveEthernet/EthernetServer.h"
#include "tm4c1294ncpdt_API/RoveEthernet/EthernetClient.h"

class EthernetServer;
class EthernetClient;

//the generic api requests the classes TcpServer and
//TcpClient, but the definitions of those classes
//can change between boards.
typedef EthernetServer TcpServer;
typedef EthernetClient TcpClient;
#elif defined(__MSP432P401R__)

typedef struct TcpServer
{
    //blank and unused on msp432
} TcpServer;
typedef struct TcpClient
{
    //blank and unused on msp432
} TcpClient;


#endif
#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_ */
