/*
 * RovePermaMem.h
 *
 *  Created on: Oct 20, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_STANDARDIZED_API_ROVEPERMAMEM_H_
#define ROVEBOARD_STANDARDIZED_API_ROVEPERMAMEM_H_

#include <stdint.h>

//write to block, get bytes per block, get first available block(whether or not to override non-empty blocks, offset), read from block,
//use block(hash), init(select bytes per block, total blocks), get total blocks, isBlockUsed
//don't actually let them reference reserved bytes. Start from 0-90 on tiva for example, internally reserve -6 to 0

typedef enum
{
  RovePermaMem_Success,
  RovePermaMem_ImproperPassword,
  RovePermaMem_AlreadyUsed
} RovePermaMem_Error;

//extern void rovePermaMem_Init(uint16_t bytesPerBlock, uint16_t blocksToAllocate); not for general layer usage
extern RovePermaMem_Error rovePermaMem_WriteBlockByte(uint16_t blockReference, uint8_t byteReference, uint8_t password, uint8_t valueToWrite);
extern RovePermaMem_Error rovePermaMem_WriteBlock(uint16_t blockReference, uint8_t password, uint8_t bytes[]);
extern RovePermaMem_Error rovePermaMem_ReadBlockByte(uint16_t blockReference, uint8_t byteReference, uint8_t password, uint8_t *readBuffer);
extern RovePermaMem_Error rovePermaMem_ReadBlock(uint16_t blockReference, uint8_t password, uint8_t byteBuffer[]);

extern RovePermaMem_Error rovePermaMem_useBlock(uint16_t blockReference, uint16_t passwordToUse);
extern uint16_t rovePermaMem_getFirstAvailableBlock(bool overrideNonEmptyBlocks, uint16_t startingBlock);
extern uint16_t rovePermaMem_getTotalAvailableBlocks();
extern uint16_t rovePermaMem_getTotalEmptyBlocks();
extern uint16_t rovePermaMem_getBytesPerBlock();
extern uint16_t rovePermaMem_getTotalBlocks();

extern RovePermaMem_Error rovePermaMem_isBlockUsed(uint16_t blockReference);
extern RovePermaMem_Error rovePermaMem_isBlockEmpty(uint16_t blockReference);

#endif /* ROVEBOARD_STANDARDIZED_API_ROVEPERMAMEM_H_ */
