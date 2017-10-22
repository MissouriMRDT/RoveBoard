/*
 * RovePermaMem_TivaTM4C1294NCPDT.cpp
 *
 *  Created on: Oct 21, 2017
 *      Author: drue
 */

#include "RovePermaMem_TivaTM4C1294NCPDT.h"
#include "RovePermaMem_Private.h"
#include "../tivaware/driverlib/eeprom.h"

//blocks 0 to 5 are reserved for roveboard stuff. Block 0 especially, being reserved for this file
const uint8_t BlockIndex_ReservedBlocks = 5;
const uint8_t BlockIndex_ControlRegister = 0;
const uint16_t BlockAddress_ControlRegister = EEPROMAddrFromBlock(BlockIndex_ControlRegister);

const uint16_t ReservedBlocksOffset = EEPROMAddrFromBlock(BlockIndex_ReservedBlocks);

const int BytesPerBlock = 64;
const int TotalBlocks = 96 - (BlockIndex_ReservedBlocks + 1); //reserved blocks is 0 index
uint16_t rovePermaMem_getBytesPerBlock()
{
  return BytesPerBlock;
}
uint16_t rovePermaMem_getTotalBlocks()
{
  return TotalBlocks;
}

