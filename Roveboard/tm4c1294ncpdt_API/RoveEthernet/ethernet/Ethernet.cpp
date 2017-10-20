#include <String.h>
#include "../../tivaware/inc/hw_ints.h"
#include "../../Clocking/Clocking_TivaTM4C1294NCPDT.h"
#include "Ethernet.h"
#include "lwip/inet.h"
#include "../../tivaware/inc/hw_flash.h"
#include "../../tivaware/inc/hw_memmap.h"
#include "../../RovePinMap_TivaTM4C1294NCPDT.h"
#include "../../tivaware/driverlib/rom.h"
#include "../../tivaware/driverlib/flash.h"
#include "../../tivaware/driverlib/interrupt.h"
#include "../../tivaware/driverlib/gpio.h"
#include "../../tivaware/driverlib/pin_map.h"
#include "../../tivaware/driverlib/emac.h"

extern void lwIPEthernetIntHandler(void);

#define ETHERNET_INT_PRIORITY   0xC0

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
	unsigned long F_CPU = getCpuClockFreq();
	uint32_t ui32User0, ui32User1;
	EMACIntRegister(EMAC0_BASE, lwIPEthernetIntHandler);
	registerSysTickCb(lwIPTimer);
	FlashUserGet(&ui32User0, &ui32User1);

	/*
	 * Convert the 24/24 split MAC address from NV ram into a 32/16 split
	 * MAC address needed to program the hardware registers, then program
	 * the MAC address into the Ethernet Controller registers.
	 */
	pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
	pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
	pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
	pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
	pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
	pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

	IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);

	if(!subnet) {
		if((local_ip >> 31) == CLASS_A)
			subnet = CLASS_A_SUBNET;
		else if((local_ip >> 30) == CLASS_B)
			subnet = CLASS_B_SUBNET;
		else if((local_ip >> 29) == CLASS_C)
			subnet = CLASS_C_SUBNET;
	}
	delayMicroseconds(100);
	lwIPInit(F_CPU, pui8MACArray, htonl(local_ip), htonl(subnet), htonl(gateway), !local_ip ? IPADDR_USE_DHCP:IPADDR_USE_STATIC);
	delayMicroseconds(100);
	lwIPDNSAddrSet((uint32_t)dns_server);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
	/* Assume the gateway will be the machine on the same network as the local IP
	 * but with last octet being '1' */
	IPAddress gateway = local_ip;
	gateway[3] = 1;
	begin(mac_address, local_ip, dns_server, gateway);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
	begin(mac_address, local_ip, dns_server, gateway, IPAddress(0,0,0,0));
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
	/* Assume the DNS server will be the machine on the same network as the local IP
	 * but with last octet being '1' */
	IPAddress dns_server = local_ip;
	dns_server[3] = 1;
	begin(mac_address, local_ip, dns_server);
}

int EthernetClass::begin(uint8_t *mac_address)
{
	begin(mac_address, IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0));
	return lwIPDHCPWaitLeaseValid();
}

int EthernetClass::maintain()
{
	/* Always return 0 which for Arduino is DHCP_CHECK_NONE */
	return 0;
}

IPAddress EthernetClass::localIP()
{
	return lwIPLocalIPAddrGet();
}

IPAddress EthernetClass::gatewayIP()
{
	return lwIPLocalGWAddrGet();
}

IPAddress EthernetClass::subnetMask()
{
	return lwIPLocalNetMaskGet();
}

IPAddress EthernetClass::dnsServerIP()
{
	return lwIPDNSAddrGet();
}

void EthernetClass::macAddress(uint8_t* mac)
{
	/* If the size of the array is not large enough 
	 * to fit the mac then simply return mac unmodified */

	memcpy(mac, pui8MACArray, 6);
}

void EthernetClass::enableLinkLed()
{
	GPIOPinConfigure(LINK_LED);
	GPIOPinTypeEthernetLED(LINK_LED_BASE, LINK_LED_PIN);
}

void EthernetClass::enableActivityLed()
{
	GPIOPinConfigure(ACTIVITY_LED);
	GPIOPinTypeEthernetLED(ACTIVITY_LED_BASE, ACTIVITY_LED_PIN);
}
