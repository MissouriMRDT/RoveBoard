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

RovePermaMem_Error rovePermaMem_WriteBlockByte(RovePermaMem_Block blockHandle, uint8_t byteReference, uint8_t valueToWrite);
RovePermaMem_Error rovePermaMem_WriteBlock(RovePermaMem_Block blockHandle, uint8_t bytes[]);
RovePermaMem_Error rovePermaMem_ReadBlockByte(RovePermaMem_Block blockHandle, uint8_t byteReference, uint8_t *readBuffer);
RovePermaMem_Error rovePermaMem_ReadBlock(RovePermaMem_Block blockHandle, uint8_t byteBuffer[]);

RovePermaMem_Block rovePermaMem_useBlock(uint16_t blockReference, uint16_t passwordToUse);
bool rovePermaMem_getFirstAvailableBlock(bool onlyGetFreshBlocks, uint16_t startingBlock, uint16_t* ret_blockReference);

uint16_t rovePermaMem_getTotalUsedBlocks();
uint16_t rovePermaMem_getTotalUnusedBlocks();
uint16_t rovePermaMem_getTotalFreshBlocks();

uint16_t rovePermaMem_getBytesPerBlock();
uint16_t rovePermaMem_getTotalBlocks();

RovePermaMem_Error rovePermaMem_isBlockUsed(uint16_t blockReference, bool *retVal);
RovePermaMem_Error rovePermaMem_isBlockFresh(uint16_t blockReference, bool *retVal);




#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_TIVATM4C1294NCPDT_H_ */
