#include "Sdc2130.h"
#include "RoveBoard.h"

Sdc2130::Sdc2130(const int pwmPin, ValueType inType, bool upsideDown): OutputDevice()
{
	PWM_PIN = pwmPin;
	inType = inType;
	invert = upsideDown;
	controlType = Pwm;
	pwmVal = 0;
	currentPower = 0;
}

void Sdc2130::move(const long movement)
{
	if(inType == InputPower)
	{
		moveSpeed(movement);
	}
	//position-input movement not implemented
	else if(inType == InputPosition)
	{

	}
}

void Sdc2130::moveSpeed(const int movement)
{
	int pow = movement;

  if(enabled) 
  {
    if(invert)
    {
      pow = -pow;
    }

    currentPower = pow;
    
    if(controlType == Pwm)
    {
      if(pow > 0)
      {
        pwmVal+=POS_INC;
      }
      else if(pow < 0)
      {
        pwmVal-=POS_INC;
      }

      if(pwmVal < PWM_MIN)
      {
        pwmVal = PWM_MIN;
      }
      else if(pwmVal > PWM_MAX)
      {
        pwmVal = PWM_MAX;
      }

      pwmWrite(PWM_PIN, pwmVal);

    }

    //serial control not implemented
    else
    {

    }
  }
}

//Tells device to behave as if it's on or off; that is, if it's off, stop and refuse to perform output until user re-enables
void Sdc2130::setPower(bool powerOn)
{
  if(powerOn == false)
  {
    stop();
  }
  
  enabled = powerOn;
}

long Sdc2130::getCurrentMove()
{
  if(inType == InputPower)
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
  else if(inType == InputPosition)
  {
    return(currentPosition);
  }
  else
  {
    return 0;
  }
}

void Sdc2130::stop()
{
  //literally nothing since we only control this by sending it commands to change positions in small increments in the first place
  currentPower = 0;
}
