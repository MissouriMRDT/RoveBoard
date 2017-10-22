/*
 * RovePermaMem.cpp
 *
 *  Created on: Oct 22, 2017
 *      Author: drue
 */
#include "RovePermaMem_Private.h"
#include "supportingUtilities/Debug.h"
#include <stdint.h>
#include "../tivaware/driverlib/eeprom.h"
#include "../tivaware/driverlib/sysctl.h"

void rovePermaMem_Init()
{
  const uint8_t wordLengthInBytes = 4;

  uint8_t controlRegister = 0;
  uint8_t initFootprintOffset = 0 * wordLengthInBytes;
  uint8_t blockInformationOffset = 1 * wordLengthInBytes;

  uint32_t writeValue;
  uint32_t readValue;

  //try turning on the eeprom
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  if(EEPROMInit() == EEPROM_INIT_ERROR)
  {
    SysCtlDelay(1000);
    if(EEPROMInit() == EEPROM_INIT_ERROR)
    {
      debugFault("EEPROM has suffered from fatal internal error. This could mean its lifespan has been exceeded");
    }
  }

  //check if this is the first the eeprom has ever been used. If so, initialize it with our startup settings.
  //Either way, by default EEPROM starts off letting us read and write to blocks that aren't password protected (and none are password
  //protected on reset) so we can just get straight to business
  EEPROMRead(&readValue, controlRegister + initFootprintOffset, wordLengthInBytes);
  if(readValue != InitFootprintKeyword)
  {

    //make sure that blockInformation word is set to the default state. Technically if this is the first time being used it'll
    //be in the default state already. Call it a sanity check
    writeValue = BlockFactoryValue;
    EEPROMProgram(&writeValue, controlRegister + blockInformationOffset, wordLengthInBytes);

    //set our footprint into the block we've allocated for it, so that we'll know on next startup that this operation was performed.
    writeValue = InitFootprintKeyword;
    EEPROMProgram(&writeValue, controlRegister + initFootprintOffset, wordLengthInBytes);
  }

  //lock all blocks in the system by writing a password to block 0 and locking it, which locks the whole system until we unlock block 0 again
  writeValue = ControlBlockPassword;
  EEPROMBlockPasswordSet(controlRegister, &writeValue, 1);
  EEPROMBlockLock(controlRegister);
}


