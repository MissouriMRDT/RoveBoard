#ifndef GENPWMPHASEHBRIDGE_H_
#define GENPWMPHASEHBRIDGE_H_

#include "AbstractFramework.h"

class GenPwmPhaseHBridge: public OutputDevice
{
  private:

    const int PWM_MIN = 0, PWM_MAX = 255;
    
    bool enableLogicHigh; //if there's an enable pin, this tracks if it's logic high or low
    int currentPower;
    unsigned int magChangeLimUp;
    unsigned int magChangeLimDown;
    bool rampUsed;

  protected:
  
    //constants for hardware GPIO pin masks
    int PWM_PIN, PHASE_PIN;
    int ENABLE_PIN; //not all general devices have an enable pin, so this pin defaults to -1
    
    //move function which passes in power ( which is converted to phase and PWM) to move device
    void move(const long movement); 
    
    //compares the user's desired power against the specified allowed amount of change per move() call, and 
    //returns a scaled power that fits within the allowed amount of change
    int scaleRamp(int desiredpower);
    
    //tells the device to power on or off. 
    //If the device class was constructed with an enable pin, the function will physically turn the device on or off
    //If it wasn't, it will virtually turn the device on or off, IE if it's off it will refuse to send an output
    void setPower(bool powerOn);
    
    //tells device to stop moving.
    void stop();
    
  public:

    //constructor for h bridge devices controlled with a pwm pin and a phase/direction pin
    //inputs: pin asignments for enable pin and phase pin, also a bool to determine the orientation of da motor
    //Pin assignment masks are based on energia pin standard
    GenPwmPhaseHBridge(const int PwmPin, const int PhPin, bool upsideDown);
    
    //constructor for h bridge devices controlled with a pwm pin and a phase/direction pin and an enable pin
    //inputs: pin asignments for enable pin and phase pin, also a bool to determine the orientation of da motor and a bool for setting 
    //if the enable pin is logic high or logic low
    //Pin assignment masks are based on energia pin standard
    GenPwmPhaseHBridge(const int PwmPin, const int PhPin, const int EnPin, bool enableLogicHigh, bool upsideDown);
    
    //sets how much the power of the device is allowed to accelerate per call of move. Default is no limit on how high it can change per call
    //input: The maximum amount the magnitude can change upward per call. If the power is requested to accelerate beyond this amount, the power is instead 
    //set to change by this amount
    void setRampUp(unsigned int magnitudeChangeLimit);
    
    //sets how much the power of the device is allowed to decelerate per call of move. Default is no limit on how high it can change per call
    //input: The maximum amount the magnitude can change downward per call. If the power is requested to decelerate beyond this amount, the power is instead 
    //set to change by this amount
    //Note that a call to setPower(false) will cause it to stop instantly
    void setRampDown(unsigned int magnitudeChangeLimit);
    
    //gets the current power value of the h bridge. 
    long getCurrentMove();
};

#endif
