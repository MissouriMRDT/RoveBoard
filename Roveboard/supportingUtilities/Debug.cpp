/*
 * Debug.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: drue
 */

#include <supportingUtilities/Debug.h>

#ifdef ROVEDEBUG_PERMISSIVE
void debugFault(char msg[])
{
}

#else

void debugFault(char msg[])
{
  while(1)
  {

  }
}
#endif



