// RoveBoard.cpp for Energia
// Author: Gbenga Osibodu

#include <supportingUtilities/Debug.h>
#include "../Clocking/Clocking_TivaTM4C1294NCPDT.h"
#include "../RoveUart/HardwareSerial.h"
#include "RoveUart_TivaTm4c1294NCPDT.h"

static HardwareSerial* uartArray[8] = {&Serial , &Serial1, &Serial2, &Serial3,
                                  &Serial4, &Serial5, &Serial6, &Serial7};

roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate, unsigned int txPin, unsigned int rxPin)
{
  return roveUartOpen(uart_index, baud_rate);
}

roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate) {
  
  uartArray[uart_index] -> begin(baud_rate);
  roveUART_Handle handle;
  handle.uart_index = uart_index;
  handle.initialized = true;

  if(uart_index > 7)
  {
    debugFault("roveUartOpen: uart index is nonsense");
  }

  return handle;
}

roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write) {
  if(uart.initialized == false)
  {
    debugFault("roveUartWrite: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];
  serial -> write((uint8_t*)write_buffer, bytes_to_write);
  serial -> flush();
    
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

int roveUartPeek(roveUART_Handle uart, uint16_t peekIndex)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartPeek: handle not initialized");
  }

  if(peekIndex == 0)
  {
    return roveUartPeek(uart);
  }
  else
  {
    HardwareSerial* serial = uartArray[uart.uart_index];
    return serial -> peek(peekIndex);
  }
}

roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read) {
  if(uart.initialized == false)
  {
    debugFault("roveUartRead: handle not initialized");
  }

  HardwareSerial* serial = uartArray[uart.uart_index];

  if (bytes_to_read == 0) {
    return ROVE_UART_ERROR_SUCCESS;
  }
  
  if (read_buffer == NULL) {
    return ROVE_UART_ERROR_UNKNOWN;
  }

  for (int i =0; i<bytes_to_read; i++) {
    while(serial -> available() == 0);
    ((unsigned char*)read_buffer)[i] = serial -> read();//Serial.println(temp[i],DEC);
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

roveUart_ERROR roveUartSettings(roveUART_Handle uart,unsigned int parityBits, unsigned int stopBits, unsigned int wordLength)
{
  if(uart.initialized == false)
  {
    debugFault("roveUartSettings: handle not initialized");
  }

	HardwareSerial* serial = uartArray[uart.uart_index];

	serial -> setOutputSettings(parityBits, stopBits, wordLength);

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