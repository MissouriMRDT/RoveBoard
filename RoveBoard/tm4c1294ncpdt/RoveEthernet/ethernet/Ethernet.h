#ifndef ethernet_h
#define ethernet_h
#include <stdbool.h>
#include "../../../utilities/IPAddress.h"
#include "stdint.h"
#include "arch/lwiplib.h"
#include "lwip/err.h"
#include "lwip/lwipopts.h"
#include "netif/tivaif.h"

#define CLASS_A 0x0
#define CLASS_B 0x2
#define CLASS_C 0x6
const IPAddress CLASS_A_SUBNET(255, 0, 0, 0);
const IPAddress CLASS_B_SUBNET(255, 255, 0, 0);
const IPAddress CLASS_C_SUBNET(255, 255, 255, 0);

class EthernetClass {
private:
	uint8_t pui8MACArray[8];
public:
	void enableLinkLed();
	void enableActivityLed();

	/* Start ethernet in DHCP mode with user MAC */
	int begin(uint8_t *mac_address);
	void begin(uint8_t *mac_address, IPAddress local_ip);
	void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server);
	void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
	void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

	/* Start ethernet in DHCP mode and use internal MAC */
	void begin() { begin(0); };
	void begin(IPAddress local_ip) { begin(0, local_ip); }
	void begin(IPAddress local_ip, IPAddress dns_server) { begin(0, local_ip, dns_server); };
	void begin(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) { begin(0, local_ip, dns_server, gateway); };
	void begin(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) { begin(0, local_ip, dns_server, gateway, subnet); };

	/* For Arduino compatibility */
	int maintain();

	/* IP Address related functions */
	IPAddress localIP();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsServerIP();

	void macAddress(uint8_t* mac);
};

extern EthernetClass Ethernet;
#endif
