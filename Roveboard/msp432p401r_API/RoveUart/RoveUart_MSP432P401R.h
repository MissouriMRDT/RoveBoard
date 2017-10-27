/*
 * RoveUart.h
 *
 *  Created on: Oct 16, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_
#define ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_

#include "../../standardized_API/RoveUart.h"

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

//sets up the specified uart to run at the specified baud rate
//inputs: index of the uart module to run (0 for uart 0, 1 for uart 1...), baud rate in bits/second (max/min ranges are board specific)
//        index of the tx pin and rx pin associated with the module (board specific).
//returns: reference to the now setup uart, for using in the other functions
roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate, unsigned int txPin, unsigned int rxPin);

//writes bytes out on a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the information to write
//(can be address of a single piece of data, an array, etc), and how many bytes are to be sent.
//returns: Information on how the process went based on roveBoard_ERROR enum
roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write);

//reads bytes from a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the buffer to read into
//(can be address of a single piece of data, an array, etc), and how many bytes are to be read.
//returns: Information on how the process went based on roveBoard_ERROR enum
//warning: Blocking, won't return until the uart has that many bytes in its incoming buffer.
roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);

//clears out the uart receive and transmit buffers
roveUart_ERROR roveUartFlushAll(roveUART_Handle uart);

//reads bytes from a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the buffer to read into
//(can be address of a single piece of data, an array, etc), and how many bytes are to be read.
//returns: Information on how the process went based on roveBoard_ERROR enum.
//Nonblocking, so if there wasn't that amount of bytes currently in the uart's incoming data buffer it returns error
roveUart_ERROR roveUartReadNonBlocking(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);

//checks how many bytes the uart currently has in its read buffer
//inputs: reference of a setup uart module from roveUartOpen
//returns: How many bytes the uart currently has in its read buffer
int roveUartAvailable(roveUART_Handle uart);

//checks what number (a byte) is at the top of the uart's read buffer, without actually taking it out of the buffer
//inputs: reference of a setup uart module from roveUartOpen
//returns: the byte at the top of the uart's read buffer
int roveUartPeek(roveUART_Handle uart);

//sets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//WARNING: If downsizing the buffer size from its previous size, any data that was currently sitting in the buffer outside of the
//length of the new buffer will be lost
void roveUartSetBufferLength(roveUART_Handle uart, uint16_t length);

//gets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//returns: roveUart receive buffer size
uint16_t roveUartGetBufferLength(roveUART_Handle uart);

//attaches a function to run whenever a module receives a message, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a receive interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is hardware specific
//warning: It's a good idea not to make these too slow to run; if the uart is constantly receiving a lot of messages, it might
//lead to the callbacks hogging the processor and even having data thrown out due to the receive FIFO being overloaded in the meantime
void roveUartAttachReceiveCb(void (*userFunc)(uint8_t moduleThatReceived));

//attaches a function to run whenever a module is finished transmitting a message or messages, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a transmit interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is hardware specific
void roveUartAttachTransmitCb(void (*userFunc)(uint8_t moduleThatTransmitted));

//modifies how many stop bits are sent and the parity settings of the uart transmissions.
//input: parityBits and stopBits are based on the the constants defined further above, any are acceptable
//returns: any errors encountered
roveUart_ERROR roveUartSettings(roveUART_Handle uart,unsigned int parityBits, unsigned int stopBits);


#endif /* ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_ */
