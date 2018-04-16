// roveBoard.h for Tiva/Energia
// Author: Gbenga Osibodu
// Second Author: Drue Satterfield
// Heavily based off of the Energia framwork's Ethernetclass and EthernetUdpclass

#include "RoveEthernet_TivaTM4C1294NCPDT.h"
#include <stdbool.h>
#include <String.h>
#include <stdint.h>
#include <stddef.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/arch/lwiplib.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/lwip/dns.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/lwip/err.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/lwip/inet.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/lwip/lwipopts.h>
#include <tm4c1294ncpdt_API/RoveEthernet/lwipLibrary/lwip/udp.h>
#include "supportingUtilities/IPAddress.h"
#include "../tivaware/inc/hw_ints.h"
#include "../Clocking/Clocking_TivaTM4C1294NCPDT.h"
#include "../tivaware/inc/hw_flash.h"
#include "../tivaware/inc/hw_memmap.h"
#include "../RovePinMap_TivaTM4C1294NCPDT.h"
#include "../tivaware/driverlib/rom.h"
#include "../tivaware/driverlib/flash.h"
#include "../tivaware/driverlib/interrupt.h"
#include "../tivaware/driverlib/gpio.h"
#include "../tivaware/driverlib/pin_map.h"
#include "../tivaware/driverlib/emac.h"

#define CLASS_A 0x0
#define CLASS_B 0x2
#define ETHERNET_INT_PRIORITY   0xC0
#define CLASS_C 0x6

#define INT_PROTECT_INIT(x)    int x = 0
#define INT_PROTECT(x)         x=IntMasterDisable()
#define INT_UNPROTECT(x)       do{if(!x)IntMasterEnable();}while(0)
#define boolean bool

static uint8_t macArray[8];
static const uint8_t MaxCallbacks = 1;
static bool (*receiveCbFuncs[MaxCallbacks])(uint8_t* msgBuffer, size_t msgSize);
static const IPAddress CLASS_A_SUBNET(255, 0, 0, 0);
static const IPAddress CLASS_B_SUBNET(255, 255, 0, 0);
static const IPAddress CLASS_C_SUBNET(255, 255, 255, 0);

struct packet {
  struct pbuf *p;
  IPAddress remoteIP;
  uint16_t remotePort;
  IPAddress destIP;
};

typedef struct
{
  struct packet packets[UDP_RX_MAX_PACKETS];
  uint8_t front;
  uint8_t rear;
  uint8_t count;

  struct udp_pcb *_pcb;
  struct pbuf *_p;
  uint16_t _port;
  /* IP and port filled in when receiving a packet */
  IPAddress _remoteIP;
  uint16_t _remotePort;
  IPAddress _destIP;
  /* pbuf, pcb, IP and port used when acting as a client */
  struct pbuf *_sendTop;
  struct udp_pcb *_sendToPcb;
  IPAddress _sendToIP;
  uint16_t _sendToPort;

  uint16_t _read;
  uint16_t _write;
}UdpData;

static UdpData data;
static bool weHazCallbacks = false;
void do_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr* addr, uint16_t port);
static bool beginUdpPacket(IPAddress ip, uint16_t port);
static size_t writeUdpPacket(const uint8_t *buffer, size_t size);
static int readUdp(unsigned char* buffer, size_t len);
static int readUdp();
static int parseUdpPacket();
static bool endUdpPacket();
static int udpAvailable();
static void stopUdp();
static void startEthernetHardware(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
static void readUdpPacket(pbuf *p, uint16_t lengthOfPacket, uint8_t* returnByteBuffer);
/*static err_t TcpServer_do_poll(void *arg, struct tcp_pcb *cpcb);
static err_t TcpServer_do_close(void *arg, struct tcp_pcb *cpcb);
static err_t TcpServer_did_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t TcpServer_do_recv(void *arg, struct tcp_pcb *cpcb, struct pbuf *p, err_t err);
err_t TcpServer_do_accept(void *arg, struct tcp_pcb *cpcb, err_t err);
static err_t TcpClient_do_connected(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t TcpClient_do_recv(void *arg, struct tcp_pcb *cpcb, struct pbuf *p, err_t err);
static err_t TcpClient_do_poll(void *arg, struct tcp_pcb *cpcb);
static void TcpClient_do_err(void * arg, err_t err);
static void TcpClient_do_dns(const char *name, struct ip_addr *ipaddr, void *arg);
static int TcpClient_ReadLocked(TcpClient *client);*/

extern void lwIPEthernetIntHandler(void);


void roveEthernet_NetworkingStart(roveIP myIP)
{
  /* Assume the DNS server will be the machine on the same network as the local IP
   * but with last octet being '1' */
  IPAddress dns_server = myIP;
  dns_server[3] = 1;

  /* Assume the gateway will be the machine on the same network as the local IP
   * but with last octet being '1' */
  IPAddress gateway = myIP;
  gateway[3] = 1;

  startEthernetHardware(myIP, dns_server, gateway, IPAddress(0,0,0,0));

  //enable link and activity led's so the users can visually see when ethernet exchange is happening
  GPIOPinConfigure(LINK_LED);
  GPIOPinTypeEthernetLED(LINK_LED_BASE, LINK_LED_PIN);
  GPIOPinConfigure(ACTIVITY_LED);
  GPIOPinTypeEthernetLED(ACTIVITY_LED_BASE, ACTIVITY_LED_PIN);
}

RoveEthernet_Error roveEthernet_UdpSocketListen(uint16_t port)
{
  data._port = port;

  if(data._pcb != 0)
  {
    stopUdp(); //catch attempts to re-init
  }

  data._pcb = udp_new();
  err_t err = udp_bind(data._pcb, IP_ADDR_ANY, port);

  if(err == ERR_USE)
    return ROVE_ETHERNET_ERROR_UNKNOWN;


  udp_recv(data._pcb, do_recv, &data);

  return ROVE_ETHERNET_ERROR_SUCCESS;
}

RoveEthernet_Error roveEthernet_EndUdpSocket()
{
  stopUdp();

  return ROVE_ETHERNET_ERROR_SUCCESS;
}

RoveEthernet_Error roveEthernet_SendUdpPacket(roveIP destIP, uint16_t destPort, const uint8_t* msg, size_t msgSize)
{
  beginUdpPacket(destIP, destPort);
  writeUdpPacket(msg, msgSize);
  endUdpPacket();
  return ROVE_ETHERNET_ERROR_SUCCESS;
}

RoveEthernet_Error roveEthernet_GetUdpMsg(roveIP* senderIP, void* buffer, size_t bufferSize)
{
  int packetSize = parseUdpPacket();
  
  if (packetSize > 0) //if there is a packet available
  {
    readUdp((unsigned char*)buffer, bufferSize);
    *senderIP = data._remoteIP;
    return ROVE_ETHERNET_ERROR_SUCCESS;
  }
  else
  {
    return ROVE_ETHERNET_ERROR_WOULD_BLOCK;
  }
}

void roveEthernet_attachUdpReceiveCb(bool (*userFunc)(uint8_t* msgBuffer, size_t msgSize))
{
  uint8_t i;
  for(i = 0; i < MaxCallbacks; i++)
  {
    if(!receiveCbFuncs[i])
    {
      receiveCbFuncs[i] = userFunc;
      break;
    }
  }

  weHazCallbacks = true;
}

static void startEthernetHardware(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
  unsigned long F_CPU = getCpuClockFreq();
  uint32_t ui32User0, ui32User1;
  EMACIntRegister(EMAC0_BASE, lwIPEthernetIntHandler);
  registerSysTickCb(lwIPTimer);
  FlashUserGet(&ui32User0, &ui32User1);

  /*
   * The tiva keeps its MAC address built into its NV ram.
   * Convert the 24/24 split MAC address from NV ram into a 32/16 split
   * MAC address needed to program the hardware registers, then program
   * the MAC address into the Ethernet Controller registers.
   */
  macArray[0] = ((ui32User0 >>  0) & 0xff);
  macArray[1] = ((ui32User0 >>  8) & 0xff);
  macArray[2] = ((ui32User0 >> 16) & 0xff);
  macArray[3] = ((ui32User1 >>  0) & 0xff);
  macArray[4] = ((ui32User1 >>  8) & 0xff);
  macArray[5] = ((ui32User1 >> 16) & 0xff);

  IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);

  if(!subnet) {
    if((local_ip >> 31) == CLASS_A)
      subnet = CLASS_A_SUBNET;
    else if((local_ip >> 30) == CLASS_B)
      subnet = CLASS_B_SUBNET;
    else if((local_ip >> 29) == CLASS_C)
      subnet = CLASS_C_SUBNET;
  }
  //delayMicroseconds(100); //causes hard faults on linux for some reason.
  lwIPInit(F_CPU, macArray, htonl(local_ip), htonl(subnet), htonl(gateway), !local_ip ? IPADDR_USE_DHCP:IPADDR_USE_STATIC);
  delayMicroseconds(100);
  lwIPDNSAddrSet((uint32_t)dns_server);
}

static bool beginUdpPacket(IPAddress ip, uint16_t port)
{
  data._sendToIP = ip;
  data._sendToPort = port;

  data._sendTop = pbuf_alloc(PBUF_TRANSPORT, UDP_TX_PACKET_MAX_SIZE, PBUF_POOL);

  data._write = 0;

  if(data._sendTop == NULL)
    return false;

  return true;
}

static size_t writeUdpPacket(const uint8_t *buffer, size_t size)
{
  uint16_t avail = data._sendTop->tot_len - data._write;

  /* If there is no more space available
   * then return immediately */
  if(avail == 0)
    return 0;

  /* If size to send is larger than is available,
   * then only send up to the space available */
  if(size > avail)
    size = avail;

  /* Copy buffer into the pbuf */
  pbuf_take(data._sendTop, buffer, size);

  data._write += size;

  return size;
}

static bool endUdpPacket()
{
  ip_addr_t dest;
  dest.addr = data._sendToIP;

  /* Shrink the pbuf to the actual size that was written to it */
  pbuf_realloc(data._sendTop, data._write);

  /* Send the buffer to the remote host */
  err_t err = udp_sendto(data._pcb, data._sendTop, &dest, data._sendToPort);

  /* udp_sendto is blocking and the pbuf is
   * no longer needed so free it */
  pbuf_free(data._sendTop);

  if(err != ERR_OK)
    return false;

  return true;
}

static int parseUdpPacket()
{
  data._read = 0;

  /* Discard the current packet */
  if(data._p) {
    pbuf_free(data._p);
    data._p = 0;
    data._remotePort = 0;
    data._remoteIP = IPAddress(IPADDR_NONE);
    data._destIP = IPAddress(IPADDR_NONE);
  }

  /* No more packets in the queue */
  if(!data.count) {
    return 0;
  }

  /* Take the next packet from the front of the queue */
  data._p = data.packets[data.front].p;
  data._remoteIP = data.packets[data.front].remoteIP;
  data._remotePort = data.packets[data.front].remotePort;
  data._destIP = data.packets[data.front].destIP;

  data.count--;

  /* Advance the front of the queue */
  data.front++;

  /* Wrap around if end of queue has been reached */
  if(data.front == UDP_RX_MAX_PACKETS)
    data.front = 0;

  /* Return the total len of the queue */
  return data._p->tot_len;
}

void do_recv(void *args, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr* addr, uint16_t port)
{
  UdpData *udp = static_cast<UdpData*>(args);

  /* No more space in the receive queue */
  if(udp->count >= UDP_RX_MAX_PACKETS) {
    pbuf_free(p);
  }
  else
  {
    bool keepPacket = true;

    //run any user attached callbacks for receiving a udp packet
    if(weHazCallbacks)
    {
      uint8_t i;
      uint16_t packetLength = p->len;
      uint8_t buff[packetLength];
      readUdpPacket(p, packetLength, buff);

      for(i = 0; i < MaxCallbacks; i++)
      {
        if(receiveCbFuncs[i])
        {
          keepPacket = receiveCbFuncs[i](buff, packetLength);
        }
      }
    }


    if(keepPacket == false)
    {
      pbuf_free(p);
    }
    else
    {
      /* Increase the number of packets in the queue
       * that are waiting for processing */
      udp->count++;
      /* Add pacekt to the rear of the queue */
      udp->packets[udp->rear].p = p;
      /* Record the IP address and port the pacekt was received from */
      udp->packets[udp->rear].remoteIP = IPAddress(addr->addr);
      udp->packets[udp->rear].remotePort = port;
      udp->packets[udp->rear].destIP = IPAddress(ip_current_dest_addr()->addr);

      /* Advance the rear of the queue */
      udp->rear++;

      /* Wrap around the end of the array was reached */
      if(udp->rear == UDP_RX_MAX_PACKETS)
        udp->rear = 0;
    }
  }
}

static int readUdp(unsigned char* buffer, size_t len)
{
  uint16_t avail = udpAvailable();
  uint16_t i;
  int b;

  if(!avail)
    return -1;

  for(i = 0; i < len; i++) {
    b = readUdp();
    if(b == -1)
      break;
    buffer[i] = b;
  }

  return i;
}

static void readUdpPacket(pbuf *p, uint16_t lengthOfPacket, uint8_t* returnByteBuffer)
{
  int i;
  uint8_t *buf = (uint8_t *)data._p->payload;

  for(i = 0; i < lengthOfPacket; i++)
  {
    if(i >=  p->len)
    {
      break;
    }

    returnByteBuffer[i] = buf[i];
  }
}

static int readUdp()
{
  if(!udpAvailable()) return -1;

  uint8_t *buf = (uint8_t *)data._p->payload;
  uint8_t b = buf[data._read];
  data._read = data._read + 1;

  if((data._read == data._p->len) && data._p->next) {
    data._read = 0;
    pbuf *p;
    p = data._p->next;
    /* Increase ref count on p->next
     * 1->2->1->etc */
    pbuf_ref(data._p->next);
    /* Free p which decreases ref count of the chain
     * and frees up to p->next in this case
     * ...->1->1->etc */
    pbuf_free(data._p);
    data._p = 0;
    data. _p = p;
  } else if(data._read == data._p->len) {
    data._read = 0;
    pbuf_free(data._p);
    data._p = 0;
  }

  return b;
}

static int udpAvailable()
{
  if(!data._p)
    return 0;

  return data._p->tot_len - data._read;
}

static void stopUdp()
{
  udp_remove(data._pcb);
  data._pcb = 0;
}

/*
TcpServer roveEthernet_TcpServer_Init(uint16_t port)
{
  TcpServer server;
  server._port = port;
  server.lastConnect = 0;
  for(int i = 0; i < MAX_CLIENTS; i++)
  {
    server.clients[i].port = 0;
  }

  return server;
}

void roveEthernet_TcpServer_SocketListen(TcpServer *server)
{
  server->spcb = tcp_new();
  tcp_bind(server->spcb, IP_ADDR_ANY, server->_port);
  server->spcb = tcp_listen(server->spcb);
  tcp_arg(server->spcb, (void*)&server);
  tcp_accept(server->spcb, TcpServer_do_accept);
}

TcpClient roveEthernet_TcpServer_Available(TcpServer *server)
{
  static uint8_t lastClient = 0; //serve the clients in a round-robin fashion
  uint8_t i;
  // Find active client
  for (i = 0; i < 1; i++) {
    if (++lastClient >= 1)
      lastClient = 0;
    if (server->clients[lastClient].port != 0) {
      //cpcb may change to NULL during interrupt servicing, so avoid the NULL pointer access
      struct tcp_pcb * cpcb = (tcp_pcb*)server->clients[lastClient].cpcb;
      if (cpcb)
      {
        if(cpcb->state == ESTABLISHED)
        {
          return roveEthernet_TcpClient_Init2(&server->clients[lastClient]);
        }
      }
    }
  }
  // No client connection active
  return roveEthernet_TcpClient_Init2(NULL);
}

err_t TcpServer_do_accept(void *arg, struct tcp_pcb *cpcb, err_t err)
{
  //
  // Get the server object from the argument
  // to get access to variables and functions
   //

  TcpServer *server = static_cast<TcpServer*>(arg);

  // Find free client //
  uint8_t i;
  for (i = 0; i < 1; i++) {
    if (server->clients[i].port == 0)
      break;
  }
  if (i >= MAX_CLIENTS) {
    return ERR_MEM;
  }

  memset(&server->clients[i], 0, sizeof(client));

  server->clients[i].port = cpcb->remote_port;
  server->clients[i].cpcb = cpcb;

  tcp_accepted(server->spcb);

  tcp_arg(cpcb, arg);
  tcp_recv(cpcb, TcpServer_do_recv);
  tcp_sent(cpcb, TcpServer_did_sent);

  //
  // Returning ERR_OK indicates to the stack the the
  // connection has been accepted
  //
  return ERR_OK;
}

size_t roveEthernet_TcpServer_Write(TcpServer *server, uint8_t byteToWrite)
{
  return roveEthernet_TcpServer_WriteBuffer(server, &byteToWrite, 1);
}

size_t roveEthernet_TcpServer_WriteBuffer(TcpServer *server, uint8_t *buf, size_t size)
{
  uint8_t i;
  size_t n = 0;
  TcpClient client;

  // Find connected clients
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (server->clients[i].port != 0 && server->clients[i].cpcb
        && server->clients[i].cpcb->state == ESTABLISHED) {
      // cpcb may change to NULL during interrupt servicing, so avoid the NULL pointer access
      struct tcp_pcb * cpcb = (tcp_pcb*)server->clients[i].cpcb;
      if (cpcb && cpcb->state == ESTABLISHED) {
        client = roveEthernet_TcpClient_Init2(&server->clients[i]);
        n += roveEthernet_TcpClient_WriteBuffer(&client, buf, size);
      }
    }
  }

  return n;
}

static err_t TcpServer_do_close(void *arg, struct tcp_pcb *cpcb)
{
  TcpServer *server = static_cast<TcpServer*>(arg);

  tcp_arg(cpcb, NULL);
  tcp_recv(cpcb, NULL);
  tcp_err(cpcb, NULL);
  tcp_poll(cpcb, NULL, 0);
  tcp_sent(cpcb, NULL);

  uint8_t i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (server->clients[i].port == cpcb->remote_port)
      break;
  }
  if (i >= MAX_CLIENTS) {
    // connection already closed
    return -1;
  }

  // --- close the connection ---

  client * cs = &server->clients[i];

  cs->read = 0;
  cs->port = 0;

  if (cs->p) {
    if (cs->cpcb)
      tcp_recved(cpcb, cs->p->tot_len);
    pbuf_free((pbuf*)cs->p);
    cs->p = NULL;
  }
  if (cs->cpcb) {
    err_t err = tcp_close(cpcb);
    if (err != ERR_OK) {
      //Error closing, try again later in polli (every 2 sec)
      tcp_poll(cpcb, TcpServer_do_poll, 4);
    }
    cs->cpcb = NULL;
  }

  return ERR_OK;
}

static err_t TcpServer_do_poll(void *arg, struct tcp_pcb *cpcb)
{
  // We only end up here if the connection failed to close
  // in an earlier call to tcp_close
  err_t err = tcp_close(cpcb);

  if (err != ERR_OK) {
    // error closing, try again later in polli (every 2 sec)
    tcp_poll(cpcb, TcpServer_do_poll, 1);
  }

  return err;
}

static err_t TcpServer_did_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  return ERR_OK;
}

static err_t TcpServer_do_recv(void *arg, struct tcp_pcb *cpcb, struct pbuf *p, err_t err)
{
  //
  // Get the server object from the argument
  // to get access to variables and functions
  //
  TcpServer *server = static_cast<TcpServer*>(arg);

  // p==0 for end-of-connection (TCP_FIN packet)
  if (p == 0) {
    TcpServer_do_close(arg, cpcb);
    return ERR_OK;
  }

  // find the connection
  uint8_t i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (server->clients[i].port == cpcb->remote_port)
      break;
  }
  if (i >= MAX_CLIENTS) {
    // connection already closed - reject the data
    return ERR_MEM;
  }

  if (server->clients[i].p != 0)
    pbuf_cat((pbuf*)server->clients[i].p, p);
  else
    server->clients[i].p = p;

  return ERR_OK;
}

TcpClient roveEthernet_TcpClient_Init()
{
  TcpClient client;
  client._connected = false;
  client.cs = &client.client_state;
  client.cs->mode = true;
  client.cs->cpcb = NULL;
  client.cs->p = NULL;

  return client;
}

TcpClient roveEthernet_TcpClient_Init2(client *c)
{
  TcpClient client;
  if (c == NULL) {
    client._connected = false;
    client.cs = &client.client_state;
    client.cs->cpcb = NULL;
    client.cs->p = NULL;
    return client;
  }
  client._connected = true;
  client.cs = c;
  client.cs->mode = false;

  return client;
}

uint8_t roveEthernet_TcpClient_Status(TcpClient *client)
{
  struct tcp_pcb * cpcb = (tcp_pcb*)client->cs->cpcb;
  if (cpcb == NULL)
    return CLOSED;
  return cpcb->state;
}

int roveEthernet_TcpClient_Connect(TcpClient *client, IPAddress ip, uint16_t port)
{
  return roveEthernet_TcpClient_ConnectTimeout(client, ip, port, CONNECTION_TIMEOUT);
}

int roveEthernet_TcpClient_ConnectHost(TcpClient *client, const char* host, uint16_t port)
{
  return roveEthernet_TcpClient_ConnectHostTimeout(client, host, port, CONNECTION_TIMEOUT);
}

int roveEthernet_TcpClient_ConnectTimeout(TcpClient *client, IPAddress ip, uint16_t port, uint64_t timeout)
{
  ip_addr_t dest;
  dest.addr = ip;

  client->cs->cpcb = tcp_new();
  client->cs->read = 0;
  client->cs->p = NULL;

  if (client->cs->cpcb == NULL) {
    return false;
  }

  tcp_arg((tcp_pcb*)client->cs->cpcb, client);
  tcp_recv((tcp_pcb*)client->cs->cpcb, TcpClient_do_recv);
  tcp_err((tcp_pcb*)client->cs->cpcb, TcpClient_do_err);

  uint8_t val = tcp_connect((tcp_pcb *)client->cs->cpcb, &dest, port, TcpClient_do_connected);

  if (val != ERR_OK) {
    return false;
  }

  // Wait for the connection.
  // Abort if the connection does not succeed within the prescribed timeout
  unsigned long then = millis();

  while (!client->_connected) {
    unsigned long now = millis();
    delay(10);
    if (now - then > timeout) {
      tcp_close((tcp_pcb*)client->cs->cpcb);
      client->cs->cpcb = NULL;
      return false;
    }
  }

  if (client->cs->cpcb->state != ESTABLISHED) {
    client->_connected = false;
  }

  // Poll to determine if the peer is still alive
  tcp_poll((tcp_pcb*)client->cs->cpcb, TcpClient_do_poll, 10);
  return client->_connected;
}
int roveEthernet_TcpClient_ConnectHostTimeout(TcpClient *client, const char* host, uint16_t port, uint64_t timeout)
{
  ip_addr_t ip;
  ip.addr = 0;

  dns_gethostbyname(host, &ip, TcpClient_do_dns, &ip);

  while (!ip.addr) {
    delay(10);
  }

  if (ip.addr == IPADDR_NONE)
    return false;

  return (roveEthernet_TcpClient_ConnectTimeout(client, IPAddress(ip.addr), port, timeout));
}

size_t roveEthernet_TcpClient_Write(TcpClient *client, uint8_t byte)
{
  return roveEthernet_TcpClient_WriteBuffer(client, &byte, 1);
}

size_t roveEthernet_TcpClient_WriteBuffer(TcpClient *client, uint8_t *buf, size_t size)
{
  uint32_t i = 0, inc = 0;
  bool stuffed_buffer = false;

  struct tcp_pcb * cpcb = (tcp_pcb*)client->cs->cpcb; // cs->cpcb may change to NULL during interrupt servicing

  if (!cpcb)
    return 0;

  // Attempt to write in 1024-byte increments.
  while (i < size) {
    inc = (size - i) < 1024 ? size - i : 1024;
    err_t err = tcp_write(cpcb, buf + i, inc, TCP_WRITE_FLAG_COPY);
    if (err != ERR_MEM) {
      // Keep enqueueing the lwIP buffer until it's full...
      i += inc;
      stuffed_buffer = false;
    } else {
      if (!stuffed_buffer) {
        // Buffer full; force output
        if (client->cs->mode)
          tcp_output(cpcb);
        stuffed_buffer = true;
      } else {
        delay(1); // else wait a little bit for lwIP to flush its buffers
      }
    }
  }
  // flush any remaining queue contents
  if (!stuffed_buffer) {
    if (client->cs->mode)
      tcp_output(cpcb);
  }

  return size;
}

int roveEthernet_TcpClient_Available(TcpClient *client)
{
  struct pbuf * p = (pbuf*)client->cs->p; // cs->p may change to NULL during interrupt servicing
  if (!p)
    return 0;
  return p->tot_len - client->cs->read;
}

int roveEthernet_TcpClient_Read(TcpClient *client)
{
  return TcpClient_ReadLocked(client);
}

int roveEthernet_TcpClient_ReadBuffer(TcpClient *client, uint8_t *buf, size_t size)
{
  uint16_t i;
  int b;

  if (roveEthernet_TcpClient_Available(client) <= 0)
    return -1;

  for (i = 0; i < size; i++)
  {
    b = roveEthernet_TcpClient_Read(client);
    if (b == -1)
      break;
    buf[i] = b;
  }

  return i;
}

int roveEthernet_TcpClient_Port(TcpClient *client)
{
  return client->cs->port;
}

IPAddress roveEthernet_TcpClient_Ip(TcpClient *client)
{
  IPAddress addr((uint32_t)(client->cs->cpcb->remote_ip.addr));

  return addr;
}

int roveEthernet_TcpClient_Peek(TcpClient *client)
{
  INT_PROTECT_INIT(oldLevel);
  uint8_t b;

  // protect code from preemption of the ethernet interrupt servicing
  INT_PROTECT(oldLevel);

  if (!roveEthernet_TcpClient_Available(client)) {
    INT_UNPROTECT(oldLevel);
    return -1;
  }

  uint8_t *buf = (uint8_t *)client->cs->p->payload;
  b = buf[client->cs->read];

  INT_UNPROTECT(oldLevel);

  return b;
}

void roveEthernet_TcpClient_Flush(TcpClient *client)
{
  INT_PROTECT_INIT(oldLevel);
  // protect code from preemption of the ethernet interrupt servicing
  INT_PROTECT(oldLevel);
  if (roveEthernet_TcpClient_Available(client)) {
    client->cs->read = client->cs->p->tot_len;
    tcp_recved((tcp_pcb*)client->cs->cpcb, 0);
  }
  INT_UNPROTECT(oldLevel);
}

void roveEthernet_TcpClient_Stop(TcpClient *client)
{
  // Stop frees any resources including any unread buffers
  err_t err;

  INT_PROTECT_INIT(oldLevel);

  // protect the code from preemption of the ethernet interrupt servicing
  INT_PROTECT(oldLevel);

  //struct tcp_pcb * cpcb_copy = (tcp_pcb *) SYNC_FETCH_AND_NULL(&cs->cpcb);
  //struct pbuf * p_copy = (pbuf *) SYNC_FETCH_AND_NULL(&cs->p); //sync shouldn't be needed, with ints disabled this is all atomic anyway

  struct tcp_pcb * cpcb_copy = (tcp_pcb *)(&client->cs->cpcb);
  struct pbuf * p_copy = (pbuf *)(&client->cs->p);
  client->cs->cpcb = NULL;
  client->cs->p = NULL;

  client->_connected = false;
  client->cs->port = 0;

  if (cpcb_copy) {
    tcp_err(cpcb_copy, NULL);

    if (p_copy) {
      tcp_recved(cpcb_copy, p_copy->tot_len);
      pbuf_free(p_copy);
    }

    err = tcp_close(cpcb_copy);

    if (err != ERR_OK) {
      // Error closing, try again later in poll (every 2 sec)
      tcp_poll(cpcb_copy, TcpClient_do_poll, 4);
    }
  }

  INT_UNPROTECT(oldLevel);
}

uint8_t roveEthernet_TcpClient_Connected(TcpClient *client)
{
  return (roveEthernet_TcpClient_Available(client) || (roveEthernet_TcpClient_Status(client) == ESTABLISHED) || client->_connected);
}

static err_t TcpClient_do_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
  TcpClient *client = static_cast<TcpClient*>(arg);

  client->_connected = true;

  return err;
}

static err_t TcpClient_do_recv(void *arg, struct tcp_pcb *cpcb, struct pbuf *p, err_t err)
{
  //
  // Get the client object from the argument
  // to get access to variables and functions
  //
  TcpClient *client = static_cast<TcpClient*>(arg);

  // p==0 for end-of-connection (TCP_FIN packet)
  if (p == 0) {
    client->_connected = false;
    return ERR_OK;
  }

  if (client->cs->p != 0)
    pbuf_cat((pbuf*)client->cs->p, p);
  else
    client->cs->p = p;

  return ERR_OK;
}

static err_t TcpClient_do_poll(void *arg, struct tcp_pcb *cpcb)
{
  TcpClient *client = static_cast<TcpClient*>(arg);

  if (client->_connected) {
    if (cpcb->keep_cnt_sent++ > 4) {
      cpcb->keep_cnt_sent = 0;
      // Stop polling
      tcp_poll(cpcb, NULL, 0);
      tcp_abort(cpcb);
      if (client->cs->cpcb == cpcb) // cs may be already re-used by another connection
      {
        client->cs->port = 0;
        client->cs->cpcb = 0;
      }
      return ERR_ABRT; // calling tcp_abort() must return ERR_ABRT
    }
    // Send tcp keep alive probe
    tcp_keepalive(cpcb);
    return ERR_OK;
  }

  // We only end up here if the connection failed to close
  // in an earlier call to tcp_close
  err_t err = tcp_close(cpcb);

  if (err != ERR_OK) {
    // error closing, try again later in poll (every 2 sec)
    tcp_poll(cpcb, TcpClient_do_poll, 4);
  }

  if (client->cs->cpcb == cpcb) // cs may be already re-used by another connection
  {
    client->cs->cpcb = 0;
    client->cs->port = 0;
  }

  return err;
}

static void TcpClient_do_err(void * arg, err_t err)
{
  TcpClient *client = static_cast<TcpClient*>(arg);

  if (client->_connected) {
    client->_connected = false;
    return;
  }

  //
  // Set connected to true to finish connecting.
  // ::connect wil take care of figuring out if we are truly connected
  // by looking at the socket state
  //
  client->_connected = true;
}

static void TcpClient_do_dns(const char *name, struct ip_addr *ipaddr, void *arg)
{
  ip_addr_t *result = (ip_addr_t *) arg;

  //BEWARE: lwip stack has been modified to set ipaddr
  // to IPADDR_NONE if the lookup failed
  result->addr = ipaddr->addr;
}

static int TcpClient_ReadLocked(TcpClient *client)
{
  if (!roveEthernet_TcpClient_Available(client)) {
    return -1;
  }

  else
  {
    // protect the code from preemption of the ethernet interrupt servicing
    IntDisable(INT_EMAC0);
  }

  uint8_t *buf = (uint8_t *) client->cs->p->payload;
  uint8_t b = buf[client->cs->read];
  client->cs->read++;

  // Indicate data was received only if still connected
  if (client->cs->cpcb) {
    tcp_recved((tcp_pcb*)client->cs->cpcb, client->cs->read);
  }

  // Read any data still in the buffer regardless of connection state
  if ((client->cs->read == client->cs->p->len) && client->cs->p->next) {
    client->cs->read = 0;
    struct pbuf * q = (pbuf*)client->cs->p;
    client->cs->p = client->cs->p->next;
    // Increase ref count on p->next
    // 1->3->1->etc
    pbuf_ref((pbuf*)client->cs->p);
    // Free p which decreases ref count of the chain
    // and frees up to p->next in this case
    // ...->1->1->etc
    pbuf_free(q);
  } else if (client->cs->read == client->cs->p->len) {
    client->cs->read = 0;
    pbuf_free((pbuf*)client->cs->p);
    client->cs->p = NULL;
  }

  IntEnable(INT_EMAC0);

  return b;
}

*/
