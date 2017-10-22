/*
 * RovePermaMem_Private.h
 *
 *  Created on: Oct 22, 2017
 *      Author: drue
 */

#ifndef ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_
#define ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_

//stuff in here is not supposed to be used by the user. Normally this stuff would just be contained in the cpp file, but
//sysinit needs some of these constants so broken out into a separate non standard include file instead

#define InitFootprintKeyword  0x0000FEFE
#define ControlBlockPassword  0x0000EFFE
#define BlockFactoryValue     0xFFFFFFFF

void rovePermaMem_Init();

#endif /* ROVEBOARD_TM4C1294NCPDT_API_ROVEPERMAMEM_ROVEPERMAMEM_PRIVATE_H_ */
