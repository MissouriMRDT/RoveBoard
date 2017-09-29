#ifndef VNH5019_H_
#define VNH5019_H_

#include "AbstractFramework.h"

class VNH5019 : public OutputDevice
{
  private:
    int PWM_PIN, INA_PIN, INB_PIN;
    const int PWM_MIN = 0, PWM_MAX = 255;
    int currentPower;

  protected:
    //move function which passes in power (which is converted to phase and PWM) to move device
    void move(const long movement); 
    
    //tells the device to power on or off. 
    void setPower(bool powerOn);
    
    //tells device to stop
    void stop();
    
  public:
  
    //pin asignments for hardware pins, also a bool to determine the orientation of the motor
    VNH5019 (const int PwmPin, const int InaPin, const int InbPin, bool upsideDown);
    
    long getCurrentMove();
};

#endif
