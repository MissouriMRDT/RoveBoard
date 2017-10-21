// Author: Gbenga Osibodu

#ifndef ROVEETHERNET_H_
#define ROVEETHERNET_H_

#include <stdint.h>
#include <stddef.h>
#include "supportingUtilities/IPAddress.h"

typedef enum
{
  ROVE_ETHERNET_ERROR_SUCCESS       =  0,
  ROVE_ETHERNET_ERROR_UNKNOWN       = -1,
  ROVE_ETHERNET_ERROR_SOCKET_IN_USE = -2,
  ROVE_ETHERNET_ERROR_HOST_DOWN     = -3,
  ROVE_ETHERNET_ERROR_WOULD_BLOCK   = -4
} roveEthernet_Error;

typedef IPAddress roveIP;

#define ROVE_IP_ADDR_NONE INADDR_NONE

extern void roveEthernet_NetworkingStart(roveIP myIP);
extern roveEthernet_Error roveEthernet_UdpSocketListen(uint16_t port);
extern roveEthernet_Error roveEthernet_SendUdpPacket(roveIP destIP, uint16_t destPort, const uint8_t* msg, size_t msgSize);
extern roveEthernet_Error roveEthernet_GetUdpMsg(roveIP* senderIP, void* buffer, size_t bufferSize);
extern roveIP roveEthernet_SetIP(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);

//attach a function to be ran automatically whenever a udp packet is received.
extern void roveEthernet_attachUdpReceiveCb(void (*userFunc)());
#endif
