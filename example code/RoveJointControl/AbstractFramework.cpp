#include "AbstractFramework.h"

bool JointInterface::verifyInput(long inputToVerify)
{
  switch(inType)
  {
	  case InputSpeed:
		return SPEED_MIN <= inputToVerify && inputToVerify <= SPEED_MAX;
	  case InputPosition:
		return POS_MIN <= inputToVerify && inputToVerify <= POS_MIN;
	  case InputPower:
		return POWER_MIN <= inputToVerify && inputToVerify <= POWER_MAX;
	  default:
	    return inputToVerify == 0;
  }
}

void IOAlgorithm::setFeedDevice(FeedbackDevice* fdDev)
{
  feedbackDev = fdDev;
  feedbackInitialized = true;
}

bool JointInterface::switchToClosedAlg(ValueType newInputType, IOAlgorithm* newAlgorithm, FeedbackDevice* newFeed)
{
  if((newInputType == newAlgorithm->inType) && (controller1->inType == newAlgorithm->outType) && (newAlgorithm->feedbackInType == newFeed->fType))
  {
    manip = newAlgorithm;
    manip->setFeedDevice(newFeed);
    inType = newInputType;
    
    algorithmUsed = true;
	
    return(true);
  }
  else
  {
    return(false);
  }
}

bool JointInterface::switchToOpenAlg(ValueType newInputType)
{
  if(newInputType == controller1->inType)
  {
    inType = newInputType;
    manip = 0;
    algorithmUsed = false;
	
    return(true);
  }
  else
  {
    return(false);
  }
}
