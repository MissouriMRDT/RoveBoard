/*
 * StructDefines.h
 *
 *  Created on: Apr 18, 2018
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_
#include "EthernetServer.h"
#include "EthernetClient.h"

class EthernetServer;
class EthernetClient;

//the generic api requests the classes TcpServer and
//TcpClient, but the definitions of those classes
//can change between boards.
typedef EthernetServer TcpServer;
typedef EthernetClient TcpClient;

#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_STRUCTDEFINES_H_ */
