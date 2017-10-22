/*
 * rovePermaMem_tivaTM4C1294NCPDT.h
 *
 *  Created on: Oct 21, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_TIVATM4C1294NCPDT_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_TIVATM4C1294NCPDT_H_

#include "standardized_API/RovePermaMem.h"
#include <stdint.h>

RovePermaMem_Error rovePermaMem_WriteBlockByte(uint16_t blockReference, uint8_t byteReference, uint8_t password, uint8_t valueToWrite);
RovePermaMem_Error rovePermaMem_WriteBlock(uint16_t blockReference, uint8_t password, uint8_t bytes[]);
RovePermaMem_Error rovePermaMem_ReadBlockByte(uint16_t blockReference, uint8_t byteReference, uint8_t password, uint8_t *readBuffer);
RovePermaMem_Error rovePermaMem_ReadBlock(uint16_t blockReference, uint8_t password, uint8_t byteBuffer[]);

RovePermaMem_Error rovePermaMem_useBlock(uint16_t blockReference, uint16_t passwordToUse);
uint16_t rovePermaMem_getFirstAvailableBlock(bool overrideNonEmptyBlocks, uint16_t startingBlock);
uint16_t rovePermaMem_getTotalAvailableBlocks();
uint16_t rovePermaMem_getTotalEmptyBlocks();

uint16_t rovePermaMem_getBytesPerBlock();
uint16_t rovePermaMem_getTotalBlocks();

RovePermaMem_Error rovePermaMem_isBlockUsed(uint16_t blockReference);
RovePermaMem_Error rovePermaMem_isBlockEmpty(uint16_t blockReference);




#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_TIVATM4C1294NCPDT_H_ */
