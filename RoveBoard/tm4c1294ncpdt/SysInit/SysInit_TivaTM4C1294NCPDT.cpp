#include <tm4c1294ncpdt/Clocking/Clocking_TivaTM4C1294NCPDT.h>
#include <tm4c1294ncpdt/SysInit/SysInit_TivaTM4C1294NCPDT.h>
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"

static bool initialized = false;

void initSystem()
{
  if(!initialized)
  {

    //Floating point computational unit is on by default in some compilers,
    //but put it here anyway just in case we use one that doesn't
    FPUEnable();
    FPULazyStackingEnable();

    //turn on system main clocks and system tick (which is used by things like delay and ethernet),
    //and turn on all GPIO ports as those are pretty much always used
    initSystemClocks();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);

    //let peripherals finish turning on
    delayMicroseconds(100);

    initialized = true;
  }
}
