#include "GenPwmPhaseHBridge.h"
#include "RoveBoard.h"

GenPwmPhaseHBridge::GenPwmPhaseHBridge(const int PwmPin, const int PhPin, bool upsideDown) : OutputDevice()
{
  PWM_PIN = PwmPin;
  PHASE_PIN = PhPin;
  invert = upsideDown;
  inType = InputPower;

  currentPower = 0;
  magChangeLimUp = (POWER_MAX - POWER_MIN);
  magChangeLimDown = (POWER_MAX - POWER_MIN);
  rampUsed = false;
  ENABLE_PIN = -1;
}

GenPwmPhaseHBridge::GenPwmPhaseHBridge(const int PwmPin, const int PhPin, const int EnPin, bool enLogicHigh, bool upsideDown) : OutputDevice()
{
  PWM_PIN = PwmPin;
  PHASE_PIN = PhPin;
  ENABLE_PIN = EnPin;
  enableLogicHigh = enLogicHigh;
  inType = InputPower;
  invert = upsideDown;

  currentPower = 0;
  magChangeLimUp = (POWER_MAX - POWER_MIN);
  magChangeLimDown = (POWER_MAX - POWER_MIN);
  rampUsed = false;
}


void GenPwmPhaseHBridge::move(const long movement)
{
  if(!enabled) return;

  int mov = invert ? -movement : movement;
  int pwm; 
  
  if(rampUsed)
  {
    mov = scaleRamp(mov);
  }
  
  //if supposed to move backwards
  if(mov < 0)
  {
    pwm = map(-mov, 0, POWER_MAX, PWM_MIN, PWM_MAX);

    //set phase to 1 for "reverse" rotation
    digitalPinWrite(PHASE_PIN, HIGH);
    
    //pulsate enable pin to control motor
    pwmWrite(PWM_PIN, pwm);
  }
  
  //if forwards
  else if(mov > 0)
  {
    pwm = map(mov, 0, POWER_MAX, PWM_MIN, PWM_MAX);
      
    //set phase to 0 for "forward" rotation
    digitalPinWrite(PHASE_PIN, LOW);
    
    //pulsate enable pin to control motor
    pwmWrite(PWM_PIN, pwm);
  }
  
  //stop
  else if(mov == 0)
  {   
    stop();
    //phase don't matter
  }
  
  currentPower = mov;
}

void GenPwmPhaseHBridge::setPower(bool powerOn)
{
  //when enable pin does not have its default value, assume device was constructed with an enable pin
  if(ENABLE_PIN != -1)
  {
    int pinNewState = ((powerOn && enableLogicHigh) || (!powerOn && !enableLogicHigh)) ? HIGH : LOW;
    
    digitalPinWrite(ENABLE_PIN, pinNewState);
  }
  
  if(!powerOn)
  {
    stop();
    currentPower = 0;
  }
  
  enabled = powerOn;
}

void GenPwmPhaseHBridge::setRampUp(unsigned int magnitudeChangeLimit)
{
  magChangeLimUp = magnitudeChangeLimit;
  rampUsed = true;
}

void GenPwmPhaseHBridge::setRampDown(unsigned int magnitudeChangeLimit)
{
  magChangeLimDown = magnitudeChangeLimit;
  rampUsed = true;
}

int GenPwmPhaseHBridge::scaleRamp(int desiredPower)
{
  bool accelerate;
  int magnitudeChangeLimit;
  
  //scale the output so that it's not allowed to change once per call more than the amount specified by magChangeLimUp or Down, for accel and decel
  if(sign(desiredPower) == sign(currentPower) || currentPower == 0) 
  {
    if(abs(desiredPower) - abs(currentPower ) >= 0)
    {
      accelerate = true;
    }
    else
    {
      accelerate = false;
    }
  }
  
  //if the destination speed and current speed are in different directions, we're decelerating until we get back to 0 speed
  else
  {
    accelerate = false;
  }
  
  if(accelerate)
  {
    magnitudeChangeLimit = magChangeLimUp;
  }
  else
  {
    magnitudeChangeLimit = magChangeLimDown;
  }
  
  //if the desired change is greater than the maximum allowed step, scale it down
  if(abs(desiredPower - currentPower) > magnitudeChangeLimit)
  {
    desiredPower = currentPower + sign(desiredPower - currentPower) * magnitudeChangeLimit;
    if(desiredPower > POWER_MAX)
    {
      desiredPower = POWER_MAX;
    }
    else if(desiredPower < POWER_MIN)
    {
      desiredPower = POWER_MIN;
    }
  }
    
  return(desiredPower);
}

long GenPwmPhaseHBridge::getCurrentMove()
{
  if(invert) //if we're inverted, then we technically move negatively even if we're moving in the 'positive' direction. The direction is the important part
  {
    return(currentPower * -1); 
  }
  else
  {
    return(currentPower);
  }
}

void GenPwmPhaseHBridge::stop()
{
  pwmWrite(PWM_PIN, 0);//set enable to 0 to brake motor
  digitalPinWrite(PHASE_PIN, LOW);
  currentPower = 0;
}
