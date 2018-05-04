/*
 * EthernetClientInternal.h
 *
 *  Created on: Apr 18, 2018
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ETHERNETCLIENTINTERNAL_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ETHERNETCLIENTINTERNAL_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * client state structure that is passed on to the client
 * through available()
 */
struct EthernetClientInternal {
  /* Connection port. (may change to 0 at any time during interrupt servicing) */
  volatile uint16_t port;
  /* Received data buffer. (may change to NULL at any time during interrupt servicing) */
  volatile struct pbuf *p;
  /* tcp control block. (may change to NULL at any time during interrupt servicing) */
  volatile struct tcp_pcb *cpcb;
  volatile bool connected;
  uint16_t read;
  bool mode;
};


#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEETHERNET_ETHERNETCLIENTINTERNAL_H_ */
