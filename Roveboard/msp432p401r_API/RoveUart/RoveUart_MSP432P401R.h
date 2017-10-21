/*
 * RoveUart.h
 *
 *  Created on: Oct 16, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_
#define ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_

#include "prototype_API/RoveUart.h"

//roveUartSettings arguments:

//constants to set stop bits in transmission
#define OneStopBit 0
#define TwoStopBit 1

//constants to set up parity bits in transmission
#define NoParity 0
#define EvenParity 1
#define OddParity 2

//uart index arguments
#define uartModule0 0
#define uartModule1 1
#define uartModule2 2
#define uartModule3 3

roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate, unsigned int txPin, unsigned int rxPin);
roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write);
roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);
roveUart_ERROR roveUartReadNonBlocking(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);
int roveUartAvailable(roveUART_Handle uart);
int roveUartPeek(roveUART_Handle uart);
roveUart_ERROR roveUartSettings(roveUART_Handle uart, unsigned int parityBits, unsigned int stopBits);

//clears out the uart receive and transmit buffers
roveUart_ERROR roveUartFlushAll(roveUART_Handle uart);

//sets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//WARNING: If downsizing the buffer size from its previous size, any data that was currently sitting in the buffer outside of the
//length of the new buffer will be lost
void roveUartSetBufferLength(roveUART_Handle uart, uint16_t length);

//gets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//returns: roveUart receive buffer size
//note: default size is 16 bytes
uint16_t roveUartGetBufferLength(roveUART_Handle uart);

//attaches a function to run whenever a module receives a message, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a receive interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is 3
void roveUartAttachReceiveCb(void (*userFunc)(uint8_t moduleThatReceived));

//attaches a function to run whenever a module is finished transmitting a message or messages, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a transmit interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is 3
void roveUartAttachTransmitCb(void (*userFunc)(uint8_t moduleThatTransmitted));



#endif /* ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_ */
