/* Programmer: Gbenga Osibodu
 * Editor/revisor: Drue Satterfield
 * Date of creation: for the original library, who knows, 2015 season I think. Reorganized into roveboard in september 2017.
 * Microcontroller used: Tiva TM4C1294NCPDT
 * Hardware components used by this file: Ethernet hardware
 *    Dependent on System Tick timer, which usually is set up in Clocking.h on the tiva to interrupt every millisecond;
 *    the clocked information that this library uses all depend on that interrupt
 *
 *
 * Description: This library is used to implement UDP networking over the tiva's ethernet port.
 *
 *
 * Warnings: There is an error in lwip where when too many packets are received at once, a memory leak occurs.
 * We encountered this problem in 2017's competition when the arm was getting 10 or so messages every 100 millis. We fixed
 * by just making it so that it gets 2 or 3 every 100 millis. The actual timeslice doesn't seem to matter, it's just how many are delivered
 * at the same time.
 */

#ifndef ROVEETHERNET_TIVATM4C1294NCPDT_H_
#define ROVEETHERNET_TIVATM4C1294NCPDT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "standardized_API/RoveEthernet.h"
#include "supportingUtilities/IPAddress.h"
#include "lwipLibrary/lwip/dns.h"

#define ROVE_IP_ADDR_NONE INADDR_NONE
#define UDP_RX_MAX_PACKETS 32
#define UDP_TX_PACKET_MAX_SIZE 2048
#define CONNECTION_TIMEOUT 1000 * 1 //milliseconds

//Start up the ethernet hardware and assign an IP to this board. Must be called before anything else
void roveEthernet_NetworkingStart(roveIP myIP);

//Start listening for UDP messages on a given port. Must be called before GetUdpMsg can be called.
RoveEthernet_Error roveEthernet_UdpSocketListen(uint16_t port);

//Stop listening for UDP messages.
RoveEthernet_Error roveEthernet_EndUdpSocket();

//Sends a udp message to the destination IP and port
RoveEthernet_Error roveEthernet_SendUdpPacket(roveIP destIP, uint16_t destPort, const uint8_t* msg, size_t msgSize);

//Checks to see if we've received a udp message since the last time this function was called and returns it if we did.
//Returns-by-pointer the IP of the sending device, and fills up an array that the user must set up themselves
//with the udp message's bytes, up to the size of the users buffer or less.
//Returns Success if there was a udp message in our receive buffer.
RoveEthernet_Error roveEthernet_GetUdpMsg(roveIP* senderIP, void* buffer, size_t bufferSize);

//attach a function to be ran automatically whenever a udp packet is received. Only one allowed. Make sure it isn't too long otherwise
//it could cause a race condition.
//function arguments:   msgBuffer[]: an array of size msgSize that contains all the data bytes that was
//                                   in the udp packet we just received
//                      msgSize:     The size of the data array
//
//function returns:     whether or not to keep this packet in the buffer. If you return true, roveEthernet will keep it in its
//                      internal udp message buffer so that it will appear again when you call GetUdpMsg, after you eventually
//                      process enough packets to bring this one to the top. False means we'll remove the packet from the
//                      buffer when the function returns and you won't see it again
//
//Note:                 Comparing to getUdpMsg: GetUdpMsg gets you the the message at the top of the buffer, this will get you the message
//                      at the bottom of the buffer. GetUdpMsg automatically removes the read packet from the buffer, this will let you choose
//                      to put it back into the buffer or remove it
void roveEthernet_attachUdpReceiveCb(bool (*userFunc)(uint8_t* msgBuffer, size_t msgSize));
/*
TcpServer roveEthernet_TcpServer_Init(uint16_t port);
void roveEthernet_TcpServer_SocketListen(TcpServer *server);
TcpClient roveEthernet_TcpServer_Available(TcpServer *server);
size_t roveEthernet_TcpServer_Write(TcpServer *server, uint8_t byteToWrite);
size_t roveEthernet_TcpServer_WriteBuffer(TcpServer *server, uint8_t *buf, size_t size);

TcpClient roveEthernet_TcpClient_Init();
TcpClient roveEthernet_TcpClient_Init2(client *c);
uint8_t roveEthernet_TcpClient_Status(TcpClient *client);
int roveEthernet_TcpClient_Connect(TcpClient *client, IPAddress ip, uint16_t port);
int roveEthernet_TcpClient_ConnectHost(TcpClient *client, const char* host, uint16_t port);
int roveEthernet_TcpClient_ConnectTimeout(TcpClient *client, IPAddress ip, uint16_t port, uint64_t timeout);
int roveEthernet_TcpClient_ConnectHostTimeout(TcpClient *client, const char* host, uint16_t port, uint64_t timeout);
size_t roveEthernet_TcpClient_Write(TcpClient *client, uint8_t byte);
size_t roveEthernet_TcpClient_WriteBuffer(TcpClient *client, uint8_t *buf, size_t size);
int roveEthernet_TcpClient_Available(TcpClient *client);
int roveEthernet_TcpClient_Read(TcpClient *client);
int roveEthernet_TcpClient_ReadBuffer(TcpClient *client, uint8_t *buf, size_t size);
int roveEthernet_TcpClient_Port(TcpClient *client);
IPAddress roveEthernet_TcpClient_Ip(TcpClient *client);
int roveEthernet_TcpClient_Peek(TcpClient *client);
void roveEthernet_TcpClient_Flush(TcpClient *client);
void roveEthernet_TcpClient_Stop(TcpClient *client);
uint8_t roveEthernet_TcpClient_Connected(TcpClient *client);*/
#endif
