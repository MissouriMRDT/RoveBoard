// Author: Gbenga Osibodu
// Second revisor: Drue Satterfield
//

#ifndef ROVEBOARD_TM4C1294NCPDT_ROVEBOARD_GENERICS_ROVEUART_H_
#define ROVEBOARD_TM4C1294NCPDT_ROVEBOARD_GENERICS_ROVEUART_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ROVE_UART_ERROR_SUCCESS = 0,
    ROVE_UART_ERROR_UNKNOWN = -1
} roveUart_ERROR;

typedef struct roveUART_Handle
{
  unsigned int uart_index;
  bool initialized;

  roveUART_Handle()
  {
    initialized = false;
  }
} roveUART_Handle;

//sets up the specified uart to run at the specified baud rate
//inputs: index of the uart module to run (0 for uart 0, 1 for uart 1...), baud rate in bits/second (max/min ranges are board specific)
//        index of the tx pin and rx pin associated with the module (board specific).
//returns: reference to the now setup uart, for using in the other functions
extern roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate, unsigned int txPin, unsigned int rxPin);

//writes bytes out on a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the information to write
//(can be address of a single piece of data, an array, etc), and how many bytes are to be sent.
//returns: Information on how the process went based on roveBoard_ERROR enum
extern roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write);

//reads bytes from a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the buffer to read into
//(can be address of a single piece of data, an array, etc), and how many bytes are to be read.
//returns: Information on how the process went based on roveBoard_ERROR enum
//warning: Blocking, won't return until the uart has that many bytes in its incoming buffer.
extern roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);

//clears out the uart receive and transmit buffers
extern roveUart_ERROR roveUartFlush(roveUART_Handle uart);

//reads bytes from a uart port
//inputs: reference of a setup uart module from roveUartOpen, a pointer to the buffer to read into
//(can be address of a single piece of data, an array, etc), and how many bytes are to be read.
//returns: Information on how the process went based on roveBoard_ERROR enum.
//Nonblocking, so if there wasn't that amount of bytes currently in the uart's incoming data buffer it returns error
extern roveUart_ERROR roveUartReadNonBlocking(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);

//checks how many bytes the uart currently has in its read buffer
//inputs: reference of a setup uart module from roveUartOpen
//returns: How many bytes the uart currently has in its read buffer
extern int roveUartAvailable(roveUART_Handle uart);

//checks what number (a byte) is at the top of the uart's read buffer, without actually taking it out of the buffer
//inputs: reference of a setup uart module from roveUartOpen
//returns: the byte at the top of the uart's read buffer
extern int roveUartPeek(roveUART_Handle uart);

//sets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//WARNING: If downsizing the buffer size from its previous size, any data that was currently sitting in the buffer outside of the
//length of the new buffer will be lost
extern void roveUartSetBufferLength(roveUART_Handle uart, uint16_t length);

//gets how many bytes the uart module is allowed to use up when saving received messages.
//inputs: reference of a setup uart module from roveUartOpen, and new length of the buffer.
//returns: roveUart receive buffer size
extern uint16_t roveUartGetBufferLength(roveUART_Handle uart);

//attaches a function to run whenever a module receives a message, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a receive interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is hardware specific
extern void roveUartAttachReceiveCb(void (*userFunc)(uint8_t moduleThatReceived));

//attaches a function to run whenever a module is finished transmitting a message or messages, on top of the functions the uart module
//itself runs internally.
//input: a function to run when a transmit interrupt is generated. The function itself should have one input that will be
//       filled with arguments by the uart module. The argument will contain the index of the module that interrupted
//note: You can usually attach multiple callbacks if desired. The maximum amount is hardware specific
extern void roveUartAttachTransmitCb(void (*userFunc)(uint8_t moduleThatTransmitted));

//for deprecated libraries
#define roveBoard_UART_open(x, y)		roveUartOpen(x, y)
#define roveBoard_UART_write(x, y, z)	roveUartWrite(x, y, z)
#define roveBoard_UART_available(x)		roveUartAvailable(x)
#define roveBoard_UART_read(x, y, z)	roveUartRead(x, y, z)

#endif
