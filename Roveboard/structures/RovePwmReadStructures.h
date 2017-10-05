/*
 * RovePwmReadStructures.h
 *
 *  Created on: Oct 4, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_STRUCTURES_ROVEPWMREADSTRUCTURES_H_
#define ROVEBOARD_STRUCTURES_ROVEPWMREADSTRUCTURES_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct rovePwmRead_Handle
{
  bool initialized;
  uint16_t mappedPin;

  rovePwmRead_Handle()
  {
    initialized = false;
  }
} rovePwmRead_Handle;



#endif /* ROVEBOARD_STRUCTURES_ROVEPWMREADSTRUCTURES_H_ */
