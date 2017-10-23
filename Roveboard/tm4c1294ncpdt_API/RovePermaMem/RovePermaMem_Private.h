/*
 * RovePermaMem_Private.h
 *
 *  Created on: Oct 22, 2017
 *      Author: drue
 */

//stuff in here is not supposed to be used by the user. Normally this stuff would just be contained in the cpp file, but
//sysinit needs some of these so broken out into a separate non standard include file instead

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_

#include <stdint.h>

#define InitFootprintKeyword  0x0000FEFE

typedef uint32_t word_t;

const uint8_t BlockReferenceTableSize = 3;
const word_t DummyMaskInLastWordOfTable = 0b11111111111111111111111111000000;
const uint32_t ControlBlockPassword = 0x0000EFFE;
const uint8_t WordLengthInBytes = 4;
static const uint8_t ControlBlock_WordOffset_blockFresh = 1 * WordLengthInBytes; //actually goes from 1 to 3
static const uint8_t ControlBlock_WordLength_blockFresh = 3 * WordLengthInBytes;

word_t blockFreshTable[BlockReferenceTableSize];
word_t blockUsedTable[BlockReferenceTableSize];

//sets up the eeprom for usage by rovePermaMem. Performs duties such as initializing the global block tables,
//setting up the eeprom for first time usage if necessary, and locking it after initialization is finished
void rovePermaMem_Init();

#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_ */
