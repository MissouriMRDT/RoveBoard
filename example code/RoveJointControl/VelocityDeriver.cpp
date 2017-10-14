#include "VelocityDeriver.h"
#include "RoveBoard.h"
#include <stdint.h>

VelocityDeriver::VelocityDeriver(FeedbackDevice* posSensor, float filter_Constant)
  : FeedbackDevice(InputSpeed), filterConstant(filter_Constant), posDev(posSensor), lastOutput(0)
{
  if(posDev->getFeedbackType() != InputPosition)
  {
    debugFault("VelocityDeriver constructor: position feedback device doesn't give position data");
  }

  filterConstant = constrain(filterConstant, 0, 1);
  lastPosition = posDev->getFeedback();
  lastTime_ms = millis();
  minimumSpeed = 1000;

  int i;
  for(i = 0; i < VelDerRingBuffSize; i++)
  {
    ringBuff[i] = 0;
    velBuff[i] = 0;
  }
}

float VelocityDeriver::accountForPositionRollover(float dP)
{
  //basically, your destination is always to the left or right of you, one way will be shorter. The direct center of the two paths is at 180 degrees
  //from your starting point. Calculate the degrees to the destination by simply taking the difference between dest and present. If it's more than 180,
  //then the shorter path is to go the other direction.
  //If the destination is actually 180 degrees from the present, then either way is technically the shortest path. : ( Defaults to positive 180
  if(abs(dP) > 180)
  {
    dP = ((360 - abs(dP)) * -1 * sign(dP));
  }
  else if(dP == -180) //use positive 180 if it's 180 degrees away
  {
    dP = 180;
  }

  return dP;
}

long VelocityDeriver::getFeedback()
{
  static float lastDpAvg = 0;
  long presentPosition = 0;
  long presentTime_ms;
  int32_t readSpeed;
  int32_t outputSpeed;
  float dT;
  float dP;
  float dPAvg;

  //output(n) = k * output(n - 1) + (1 - k) * input(n)
  presentPosition = posDev->getFeedback();
  presentTime_ms = millis();

  dP = presentPosition - lastPosition;
  dP = accountForPositionRollover(dP);

  dPAvg = getDpAvg(dP);

  dPAvg = .75 * lastDpAvg + (1.0 - .75) * dPAvg;

  dT = presentTime_ms - lastTime_ms;
  dT /= 1000.0;

  if(dT <= 0)
  {
    return lastOutput;
  }

  readSpeed = dPAvg/dT;

  outputSpeed = getSpeedAvg(readSpeed);

  //output(n) = k * output(n - 1) + (1 - k) * input(n)
  outputSpeed = filterConstant * lastOutput + (1.0 - filterConstant) * outputSpeed;
  if(abs(outputSpeed) < minimumSpeed)
  {
    outputSpeed = 0;
  }

  lastOutput = outputSpeed;
  lastTime_ms = presentTime_ms;
  lastPosition = presentPosition;
  lastDpAvg = dPAvg;

  return outputSpeed;
}

float VelocityDeriver::getDpAvg(float dP)
{
  float avgDp = 0;
  int i;
  for(i = VelDerRingBuffSize - 1; i > 0; i--)
  {
    ringBuff[i] = ringBuff[i - 1];
  }

  ringBuff[0] = dP;

  for(i = 0; i < VelDerRingBuffSize; i++)
  {
    avgDp += ringBuff[i];
  }

  avgDp /=VelDerRingBuffSize;

  return avgDp;
}

float VelocityDeriver::getSpeedAvg(float speed)
{
  float avg = 0;
  int i;
  for(i = VelDerRingBuffSize - 1; i > 0; i--)
  {
    velBuff[i] = velBuff[i - 1];
  }

  velBuff[0] = speed;

  for(i = 0; i < VelDerRingBuffSize; i++)
  {
    avg += velBuff[i];
  }

  avg /=VelDerRingBuffSize;

  return avg;
}
