/*
 ************************************************************************
 *	HardwareSerial.cpp
 *
 *	Arduino core files for ARM Cortex-M4F: Tiva-C and Stellaris
 *		Copyright (c) 2012 Robert Wessels. All right reserved.
 *
 *
 ***********************************************************************
 2013-12-23 Limited size for RX and TX buffers, by spirilis

  Derived from:
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 9/3/17 by drue satterfield, who took it wholesale for roveboard
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "../Clocking/Clocking_TivaTM4C1294NCPDT.h"
#include "HardwareSerial.h"
#include "../tivaware/inc/hw_memmap.h"
#include "../tivaware/inc/hw_types.h"
#include "../tivaware/inc/hw_ints.h"
#include "../tivaware/inc/hw_uart.h"
#include "../tivaware/driverlib/gpio.h"
#include "../tivaware/driverlib/debug.h"
#include "../tivaware/driverlib/interrupt.h"
#include "../tivaware/driverlib/rom.h"
#include "../tivaware/driverlib/rom_map.h"
#include "../tivaware/driverlib/pin_map.h"
#include "../tivaware/driverlib/sysctl.h"
#include "../tivaware/driverlib/uart.h"

#define TX_BUFFER_EMPTY    (txReadIndex == txWriteIndex)
#define TX_BUFFER_FULL     (((txWriteIndex + 1) % txBufferSize) == txReadIndex)

#define RX_BUFFER_EMPTY    (rxReadIndex == rxWriteIndex)
#define RX_BUFFER_FULL     (((rxWriteIndex + 1) % rxBufferSize) == rxReadIndex)

#define UART_BASE g_ulUARTBase[uartModule]

static const uint8_t MaxCallbacks = 8;
static void (*receiveCbFuncs[MaxCallbacks])(uint8_t module);
static void (*transmitCbFuncs[MaxCallbacks])(uint8_t module);

static const unsigned long g_ulUARTBase[8] =
{
    UART0_BASE, UART1_BASE, UART2_BASE, UART3_BASE,
	UART4_BASE, UART5_BASE, UART6_BASE, UART7_BASE
};

//*****************************************************************************
//
// The list of possible interrupts for the console UART.
//
//*****************************************************************************
static const unsigned long g_ulUARTInt[8] =
{
    INT_UART0, INT_UART1, INT_UART2, INT_UART3,
	INT_UART4, INT_UART5, INT_UART6, INT_UART7
};

//*****************************************************************************
//
// The list of UART peripherals.
//
//*****************************************************************************
static const unsigned long g_ulUARTPeriph[8] =
{
    SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2,
	SYSCTL_PERIPH_UART3, SYSCTL_PERIPH_UART4, SYSCTL_PERIPH_UART5,
	SYSCTL_PERIPH_UART6, SYSCTL_PERIPH_UART7
};
//*****************************************************************************
//
// The list of UART GPIO configurations.
//
//*****************************************************************************
static const unsigned long g_ulUARTConfig[8][2] =
{

    {GPIO_PA0_U0RX, GPIO_PA1_U0TX}, {GPIO_PB0_U1RX, GPIO_PB1_U1TX},
    {GPIO_PA6_U2RX, GPIO_PA7_U2TX}, {GPIO_PA4_U3RX, GPIO_PA5_U3TX},
    {GPIO_PK0_U4RX, GPIO_PK1_U4TX},	{GPIO_PC6_U5RX, GPIO_PC7_U5TX},
	{GPIO_PP0_U6RX, GPIO_PP1_U6TX},	{GPIO_PC4_U7RX, GPIO_PC5_U7TX}

};

static const unsigned long g_ulUARTPort[8] =
{
#if defined(PART_TM4C1233H6PM) || defined(PART_LM4F120H5QR)
	GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTD_BASE, GPIO_PORTC_BASE,
	GPIO_PORTC_BASE, GPIO_PORTE_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE
#elif defined(PART_TM4C129XNCZAD)
	GPIO_PORTA_BASE, GPIO_PORTQ_BASE, GPIO_PORTD_BASE, GPIO_PORTA_BASE,
	GPIO_PORTK_BASE, GPIO_PORTH_BASE, GPIO_PORTP_BASE, GPIO_PORTC_BASE
#elif defined(PART_TM4C1294NCPDT)
	GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTA_BASE, GPIO_PORTA_BASE,
	GPIO_PORTK_BASE, GPIO_PORTC_BASE, GPIO_PORTP_BASE, GPIO_PORTC_BASE
#else
#error "**** No PART defined or unsupported PART ****"
#endif
};

static const unsigned long g_ulUARTPins[8] =
{

    GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1,
    GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4 | GPIO_PIN_5,
    GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_6 | GPIO_PIN_7,
    GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_4 | GPIO_PIN_5

};

// Constructors ////////////////////////////////////////////////////////////////
HardwareSerial::HardwareSerial(void)
{
    txWriteIndex = 0;
    txReadIndex = 0;
    rxWriteIndex = 0;
    rxReadIndex = 0;
    uartModule = 0;

    txBuffer = (unsigned char *) 0xFFFFFFFF;
    rxBuffer = (unsigned char *) 0xFFFFFFFF;
    txBufferSize = SERIAL_BUFFER_SIZE;
    rxBufferSize = SERIAL_BUFFER_SIZE;
}

HardwareSerial::HardwareSerial(unsigned long module)
{
    txWriteIndex = 0;
    txReadIndex = 0;
    rxWriteIndex = 0;
    rxReadIndex = 0;
    uartModule = module;

    txBuffer = (unsigned char *) 0xFFFFFFFF;
    rxBuffer = (unsigned char *) 0xFFFFFFFF;
    txBufferSize = SERIAL_BUFFER_SIZE;
    rxBufferSize = SERIAL_BUFFER_SIZE;
}
// Private Methods //////////////////////////////////////////////////////////////
void
HardwareSerial::flushAll(void)
{
    // wait for transmission of outgoing data
    while(!TX_BUFFER_EMPTY)
    {
    }
    while (UARTBusy(UART_BASE)) ;
    txReadIndex = 0;
    txWriteIndex = 0;

    //
    // Flush the receive buffer.
    //
    rxReadIndex = 0;
    rxWriteIndex = 0;
}

void
HardwareSerial::primeTransmit(unsigned long ulBase)
{
    //
    // Do we have any data to transmit?
    //
    if(!TX_BUFFER_EMPTY)
    {
        //
        // Disable the UART interrupt. If we don't do this there is a race
        // condition which can cause the read index to be corrupted.
        //
        IntDisable(g_ulUARTInt[uartModule]);
        //
        // Yes - take some characters out of the transmit buffer and feed
        // them to the UART transmit FIFO.
        //
        while(!TX_BUFFER_EMPTY)
        {
            while(UARTSpaceAvail(ulBase) && !TX_BUFFER_EMPTY){
                UARTCharPutNonBlocking(ulBase,
                                       txBuffer[txReadIndex]);

                txReadIndex = (txReadIndex + 1) % txBufferSize;
            }
        }

        //
        // Reenable the UART interrupt.
        //
        IntEnable(g_ulUARTInt[uartModule]);
    }
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::setOutputSettings(uint8_t paritySettings, uint8_t stopBitSettings, uint8_t wordLengthSettings)
{
	uint32_t wordLength, stopBits, parityBits;
	switch(paritySettings)
	{
		case NoParity:
			parityBits = UART_CONFIG_PAR_NONE;
			break;

		case EvenParity:
			parityBits = UART_CONFIG_PAR_EVEN;
			break;

		case OddParity:
			parityBits = UART_CONFIG_PAR_ODD;
			break;

		case AlwaysZero:
			parityBits = UART_CONFIG_PAR_ZERO;
			break;

		case AlwaysOne:
			parityBits = UART_CONFIG_PAR_ONE;
			break;

		default:
			return;
	}

	switch(stopBitSettings)
	{
		case OneStopBit:
			stopBits = UART_CONFIG_STOP_ONE;
			break;

		case TwoStopBit:
			stopBits = UART_CONFIG_STOP_TWO;
			break;

		default:
			return;
	}

	switch(wordLengthSettings)
	{
		case WordLength8:
			wordLength = UART_CONFIG_WLEN_8;
			break;

		case WordLength7:
			wordLength = UART_CONFIG_WLEN_7;
			break;

		case WordLength6:
			wordLength = UART_CONFIG_WLEN_6;
			break;

		case WordLength5:
			wordLength = UART_CONFIG_WLEN_5;
			break;

		default:
			return;
	}

	//function call automatically stops uart for configuration
	UARTConfigSetExpClk(UART_BASE, getCpuClockFreq(), baudRate, (wordLength | parityBits |stopBits));
}

void
HardwareSerial::begin(unsigned long baud)
{
	baudRate = baud;
    //
    // Initialize the UART.
    //
    SysCtlPeripheralEnable(g_ulUARTPeriph[uartModule]);

    GPIOPinConfigure(g_ulUARTConfig[uartModule][0]);
    GPIOPinConfigure(g_ulUARTConfig[uartModule][1]);

    GPIOPinTypeUART(g_ulUARTPort[uartModule], g_ulUARTPins[uartModule]);

    UARTConfigSetExpClk(UART_BASE, getCpuClockFreq(), baudRate,
                            (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_WLEN_8));

    switch(UART_BASE)
    {
    	case UART0_BASE:
    		UARTIntRegister(UART_BASE, UARTIntHandler0);
			break;

    	case UART1_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler1);
			break;

    	case UART2_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler2);
			break;

    	case UART3_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler3);
			break;

    	case UART4_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler4);
			break;

    	case UART5_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler5);
			break;

    	case UART6_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler6);
			break;

    	case UART7_BASE:
			UARTIntRegister(UART_BASE, UARTIntHandler7);
			break;
    }

    //
    // Set the UART to interrupt whenever a byte is transmitted or received
    //
    UARTFIFOLevelSet(UART_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    flushAll();
    UARTIntDisable(UART_BASE, 0xFFFFFFFF);
    UARTIntEnable(UART_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(g_ulUARTInt[uartModule]);

    //
    // Enable the UART operation.
    //
    UARTEnable(UART_BASE);

    if (txBuffer != (unsigned char *)0xFFFFFFFF)  // Catch attempts to re-init this Serial instance by freeing old buffer first
      delete txBuffer;
    if (rxBuffer != (unsigned char *)0xFFFFFFFF)  // Catch attempts to re-init this Serial instance by freeing old buffer first
      delete rxBuffer;
    txBuffer = new unsigned char [txBufferSize];
    rxBuffer = new unsigned char [rxBufferSize];

    SysCtlDelay(100);
}

void
HardwareSerial::setBufferSize(unsigned long buffSize)
{
  if(buffSize == 0)
  {
    return;
  }

  unsigned char* oldRxBuffer = rxBuffer;
  unsigned char* oldTxBuffer = txBuffer;
  unsigned long oldBuffSize = txBufferSize;

  IntDisable(g_ulUARTInt[uartModule]);

  txBuffer = new unsigned char [buffSize];
  rxBuffer = new unsigned char [buffSize];

  unsigned long i;
  for(i = 0; i < oldBuffSize; i++)
  {
    if(i >= buffSize)
    {
      break;
    }

    rxBuffer[i] = oldRxBuffer[i];
    txBuffer[i] = oldTxBuffer[i];
  }

  txBufferSize = buffSize;
  rxBufferSize = buffSize;
  delete oldRxBuffer;
  delete oldTxBuffer;

  IntEnable(g_ulUARTInt[uartModule]);
}

unsigned long HardwareSerial::getBufferSize()
{
  return txBufferSize; //rx, tx share same buff size
}

void
HardwareSerial::setModule(unsigned long module)
{
    UARTIntDisable(UART_BASE, UART_INT_RX | UART_INT_RT);
    IntDisable(g_ulUARTInt[uartModule]);
	uartModule = module;
	begin(baudRate);

}
void
HardwareSerial::setPins(unsigned long pins)
{
	if(pins == UART1_PORTB)
	{
		GPIOPinConfigure(GPIO_PB0_U1RX);
		GPIOPinConfigure(GPIO_PB1_U1TX);
		GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	}
	else
	{
		//Default UART1 Pin Muxing
		GPIOPinConfigure(g_ulUARTConfig[1][0]);
		GPIOPinConfigure(g_ulUARTConfig[1][1]);
		GPIOPinTypeUART(g_ulUARTPort[1], g_ulUARTPins[1]);
	}
}

HardwareSerial::operator bool()
{
	return true;
}

void HardwareSerial::end()
{
    unsigned long ulInt = IntMasterDisable();

	flushAll();

    //
    // If interrupts were enabled when we turned them off, turn them
    // back on again.
    //
    if(!ulInt)
    {
        IntMasterEnable();
    }

    IntDisable(g_ulUARTInt[uartModule]);
    UARTIntDisable(UART_BASE, UART_INT_RX | UART_INT_RT);
}

int HardwareSerial::available(void)
{
    return((rxWriteIndex >= rxReadIndex) ?
		(rxWriteIndex - rxReadIndex) : rxBufferSize - (rxReadIndex - rxWriteIndex));
}

int HardwareSerial::peek(uint16_t index)
{
  uint8_t cChar;

  //
  // Check to see if there's anything to get
  //
  if(RX_BUFFER_EMPTY || index >= available())
  {
    return -1;
  }

  uint16_t tempRead = ((rxReadIndex) + index) % rxBufferSize;

  //
  // Read a character from the buffer.
  //
  cChar = rxBuffer[tempRead];
  //
  // Return the character to the caller.
  //
  return(cChar);
}

int HardwareSerial::peek(void)
{
    unsigned char cChar = 0;

    //
    // Wait for a character to be received.
    //
    if(RX_BUFFER_EMPTY)
    {
    	return -1;
    	//
        // Block waiting for a character to be received (if the buffer is
        // currently empty).
        //
    }

    //
    // Read a character from the buffer.
    //
    cChar = rxBuffer[rxReadIndex];
    //
    // Return the character to the caller.
    //
    return(cChar);
}

int HardwareSerial::read(void)
{
    if(RX_BUFFER_EMPTY) {
    	return -1;
    }

    //
    // Read a character from the buffer.
    //
    unsigned char cChar = rxBuffer[rxReadIndex];
	rxReadIndex = ((rxReadIndex) + 1) % rxBufferSize;
	return cChar;
}

void HardwareSerial::flush()
{
    while(!TX_BUFFER_EMPTY);
    while (UARTBusy(UART_BASE)) ;
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
	n += write(*buffer++);
  }
  return n;
}

size_t HardwareSerial::println(const char* str)
{
  int i = 0;
  size_t cnt = 0;
  while(str[i] != 0)
  {
    write(str[i]);
    cnt++;
  }

  return cnt;
}

size_t HardwareSerial::write(uint8_t c)
{

    unsigned int numTransmit = 0;
    //
    // Check for valid arguments.
    //
    ASSERT(c != 0);

    //
    // Send the character to the UART output.
    //
    while (TX_BUFFER_FULL);
    txBuffer[txWriteIndex] = c;
    txWriteIndex = (txWriteIndex + 1) % txBufferSize;
    numTransmit ++;

    //
    // If we have anything in the buffer, make sure that the UART is set
    // up to transmit it.
    //
    if(!TX_BUFFER_EMPTY)
    {
	    primeTransmit(UART_BASE);
      UARTIntEnable(UART_BASE, UART_INT_TX);
    }

    //
    // Return the number of characters written.
    //
    return(numTransmit);
}

void HardwareSerial::UARTIntHandler(void){
    unsigned long ulInts;
    long lChar;
    // Get and clear the current interrupt source(s)
    //
    ulInts = UARTIntStatus(UART_BASE, true);
    UARTIntClear(UART_BASE, ulInts);

    // Are we being interrupted because the TX FIFO has space available?
    //
    if(ulInts & UART_INT_TX)
    {
        //
        // Move as many bytes as we can into the transmit FIFO.
        //
        primeTransmit(UART_BASE);

        //
        // If the output buffer is empty, turn off the transmit interrupt. Run any user transmit callbacks
        //
        if(TX_BUFFER_EMPTY)
        {
            UARTIntDisable(UART_BASE, UART_INT_TX);
            uint8_t i;
            for(i = 0; i < MaxCallbacks; i++)
            {
              if(transmitCbFuncs[i])
              {
                transmitCbFuncs[i](uartModule);
              }
            }
        }
    }
    if(ulInts & (UART_INT_RX | UART_INT_RT))
    {
        while(UARTCharsAvail(UART_BASE))
        {

            //
            // Read a character
            //
            lChar = UARTCharGetNonBlocking(UART_BASE);
            //
            // If there is space in the receive buffer, put the character
            // there, otherwise throw it away.
            //
            uint8_t volatile full = RX_BUFFER_FULL;
            if(full) break;

            rxBuffer[rxWriteIndex] =
                (unsigned char)(lChar & 0xFF);
            rxWriteIndex = ((rxWriteIndex) + 1) % rxBufferSize;

            //
            // call user receive callbacks
            //
            uint8_t i;
            for(i = 0; i < MaxCallbacks; i++)
            {
              if(receiveCbFuncs[i])
              {
                receiveCbFuncs[i](uartModule);
              }
            }

        }
        primeTransmit(UART_BASE);
        UARTIntEnable(UART_BASE, UART_INT_TX);
    }
}

void attachTransmitCb(void (*userFunc)(uint8_t))
{
  uint8_t i;
  for(i = 0; i < MaxCallbacks; i++)
  {
    if(!transmitCbFuncs[i])
    {
      transmitCbFuncs[i] = userFunc;
      break;
    }
  }
}

void attachReceiveCb(void (*userFunc)(uint8_t))
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
}

void
UARTIntHandler0(void)
{
    Serial.UARTIntHandler();
}

void
UARTIntHandler1(void)
{
    Serial1.UARTIntHandler();
}

void
UARTIntHandler2(void)
{
    Serial2.UARTIntHandler();
}

void
UARTIntHandler3(void)
{
    Serial3.UARTIntHandler();
}

void
UARTIntHandler4(void)
{
    Serial4.UARTIntHandler();
}

void
UARTIntHandler5(void)
{
    Serial5.UARTIntHandler();
}

void
UARTIntHandler6(void)
{
    Serial6.UARTIntHandler();
}

void
UARTIntHandler7(void)
{
    Serial7.UARTIntHandler();
}

HardwareSerial Serial;
HardwareSerial Serial1(1);
HardwareSerial Serial2(2);
HardwareSerial Serial3(3);
HardwareSerial Serial4(4);
HardwareSerial Serial5(5);
HardwareSerial Serial6(6);
HardwareSerial Serial7(7);
