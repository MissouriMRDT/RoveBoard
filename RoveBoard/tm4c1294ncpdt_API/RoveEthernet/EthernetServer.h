#ifndef ethernetserver_h
#define ethernetserver_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lwipLibrary/lwip/tcp.h"
#include "EthernetClient.h"
#include "EthernetClientInternal.h"
#include "ConfigDefines.h"

class EthernetClient;

class EthernetServer{
private:
	unsigned long lastConnect;
	uint16_t _port;
	struct tcp_pcb *spcb;
	struct EthernetClientInternal clients[MAX_CLIENTS];
	static err_t do_poll(void *arg, struct tcp_pcb *cpcb);
	static void  do_close(void *arg, struct tcp_pcb *cpcb);
public:
	EthernetServer();
	EthernetClient available();
	virtual void begin(uint16_t);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	static err_t do_accept(void *arg, struct tcp_pcb *pcb, err_t err);
	static err_t do_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
	static err_t did_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
};


#endif
