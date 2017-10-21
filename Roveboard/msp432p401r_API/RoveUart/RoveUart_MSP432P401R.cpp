/*
 * RoveUart.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: drue
 */

#include "RoveUart_MSP432P401R.h"
#include "../msp432Ware/inc/msp.h"

#include "HardwareSerial.h"
#include "../RovePinMap_MSP432P401R.h"
#include "../../supportingUtilities/Debug.h"

static HardwareSerial* uartArray[] = {&Serial , &Serial1, &Serial2, &Serial3};
static const char MaxBytesPerWrite = 16;

static bool validatePins(char module, char txPin, char rxPin)
{
  switch(module)
  {
    case uartModule0:
      if(rxPin == P1_2 && txPin == P1_3)
      {
        return true;
      }
      else
      {
        return false;
      }

    case uartModule1:
      if(rxPin == P2_2 && txPin == P2_3)
      {
        return true;
      }
      else
      {
        return false;
      }

    case uartModule2:
      if(rxPin == P3_2 && txPin == P3_3)
      {
        return true;
      }
      else
      {
        return false;
      }

    case uartModule3:
      if(rxPin == P9_6 && txPin == P9_7)
      {
        return true;
      }
      else
      {
        return false;
      }
  }

  return false;
}

roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate, unsigned int txPin, unsigned int rxPin) {

  if(uart_index > 3)
  {
    debugFault("roveUartOpen: uart index is nonsense");
  }
  else if(baud_rate >= MAP_CS_getHSMCLK())
  {
    debugFault("roveUartOpen: baud rate is greater than or equal to the peripheral clock source frequency");
  }
  else if(validatePins(uart_index, txPin, rxPin) == false)
  {
    debugFault("roveUartOpen: pins are nonsense");
  }

  uartArray[uart_index] -> begin(baud_rate, txPin, rxPin);
  roveUART_Handle handle;
  handle.uart_index = uart_index;
  handle.initialized = true;

  return handle;
}

roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write) {
  if(uart.initialized == false)
  {
    debugFault("roveUartWrite: handle not initialized");
  }

  size_t packetSize;
  HardwareSerial* serial = uartArray[uart.uart_index];

  while(bytes_to_write > 0)
  {
    if(bytes_to_write <= MaxBytesPerWrite)
    {
      packetSize = bytes_to_write;
    }
    else
    {
      packetSize = MaxBytesPerWrite;
    }

    serial -> write((uint8_t*)write_buffer, packetSize);
    serial -> flush();

    bytes_to_write -= packetSize;
  }

  return ROVE_UART_ERROR_SUCCESS;
}

int roveUartPeek(roveUART_Handle uart)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartPeek: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  return serial -> peek();
}

roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read) {
  if(uart.initialized == false)
  {
    debugFault("roveUartRead: handle not initialized");
  }
  else if (bytes_to_read == 0)
  {
    return ROVE_UART_ERROR_SUCCESS;
  }
  else if (read_buffer == NULL)
  {
    return ROVE_UART_ERROR_UNKNOWN;
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  for (int i =0; i<bytes_to_read; i++) {
    while(serial -> available() == 0);
    ((unsigned char*)read_buffer)[i] = serial -> read();
  }

  return ROVE_UART_ERROR_SUCCESS;
}

roveUart_ERROR roveUartReadNonBlocking(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read)
{
  if(roveUartAvailable(uart) < bytes_to_read)
  {
    return ROVE_UART_ERROR_UNKNOWN;
  }
  else
  {
    return roveUartRead(uart, read_buffer, bytes_to_read);
  }
}

int roveUartAvailable(roveUART_Handle uart) {
  if(uart.initialized == false)
  {
    debugFault("roveUartAvailable: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  return serial -> available();
}

roveUart_ERROR roveUartFlushAll(roveUART_Handle uart)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartFlushAll: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  serial->flushAll();

  return ROVE_UART_ERROR_SUCCESS;
}

roveUart_ERROR roveUartSettings(roveUART_Handle uart,unsigned int parityBits, unsigned int stopBits)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartSettings: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  serial -> setOutputSettings(parityBits, stopBits);

  return ROVE_UART_ERROR_SUCCESS;
}

void roveUartSetBufferLength(roveUART_Handle uart, uint16_t length)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartSetFIFOLength: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  serial->setBufferSize(length);
}

uint16_t roveUartGetBufferLength(roveUART_Handle uart)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartGetFIFOLength: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  return serial->getBufferSize();
}

void roveUartAttachReceiveCb(void (*userFunc)(uint8_t moduleThatReceived))
{
  attachTransmitCb(userFunc);
}

void roveUartAttachTransmitCb(void (*userFunc)(uint8_t moduleThatTransmitted))
{
  attachReceiveCb(userFunc);
}


