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

roveUART_Handle roveUartOpen(unsigned int uart_index, unsigned int baud_rate);
roveUart_ERROR roveUartWrite(roveUART_Handle uart, void* write_buffer, size_t bytes_to_write);
roveUart_ERROR roveUartRead(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);
roveUart_ERROR roveUartReadNonBlocking(roveUART_Handle uart, void* read_buffer, size_t bytes_to_read);
int roveUartAvailable(roveUART_Handle uart);
int roveUartPeek(roveUART_Handle uart);
roveUart_ERROR roveUartSettings(roveUART_Handle uart, unsigned int parityBits, unsigned int stopBits);



#endif /* ROVEBOARD_MSP432P401R_API_ROVEUART_ROVEUART_MSP432P401R_H_ */
