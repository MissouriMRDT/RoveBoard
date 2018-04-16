/*
 * RoveEthernet_StructDefinitions.h
 *
 *  Created on: Apr 15, 2018
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ROVEETHERNET_STRUCTDEFINITIONS_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ROVEETHERNET_STRUCTDEFINITIONS_H_

#include <stdint.h>
#include <stddef.h>

#define TcpClient TcpClient_tm4c
#define TcpServer TcpServer_tm4c
#define client client_tm4c
#define MAX_CLIENTS 4

//used to support the other structs by containing inner
//information about clients
typedef struct{
  /* Connection port. (may change to 0 at any time during interrupt servicing) */
  volatile uint16_t port;
  /* Received data buffer. (may change to NULL at any time during interrupt servicing) */
  volatile struct pbuf *p;
  /* tcp control block. (may change to NULL at any time during interrupt servicing) */
  volatile struct tcp_pcb *cpcb;
  volatile bool connected;
  uint16_t read;
  bool mode;
} client_tm4c;

typedef struct
{
    uint8_t lastConnect;
    uint16_t _port;
    struct tcp_pcb *spcb;
    client_tm4c clients[MAX_CLIENTS];
} TcpServer_tm4c;

typedef struct
{
    client_tm4c client_state;
    volatile bool _connected;
    client_tm4c *cs;
} TcpClient_tm4c;



#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ROVEETHERNET_STRUCTDEFINITIONS_H_ */
