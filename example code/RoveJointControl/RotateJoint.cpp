#include "RotateJoint.h"
#include "TiltJoint.h"

//constructor for the rotate joint class without feedback.
//Constructor automatically chooses an open loop algorithm that inputs the specified inputType and outputs the values the output device accepts
//Assumes both passed devices have the same input type
//inputType: What kind of movement this joint should be controlled by, such as speed or position input.
//cont1: The first output device controlling the first motor on this joint
//cont2: The second output device controlling the second motor on this joint
RotateJoint::RotateJoint(ValueType inputType, OutputDevice* cont1, OutputDevice* cont2) : JointInterface()
{
	//assignments
	inType = inputType;
	controller1 = cont1;
	controller2 = cont2;
  coupled = false;
  motorOneVirtualPower = 0;
  motorTwoVirtualPower = 0;
	algorithmUsed = false;

  //checks to make sure the passed arguments all work with each other, that is that the algorithm's input type is the same as what the user is putting in, and
  //that the algorithm's output value type is what the output device expects to take in, both output devices line up properly, etc
  if((inputType == cont1->inType) && (cont1->inType == cont2->inType))
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

//constructor for the rotate joint class with feedback.
//Assumes both passed devices have the same input type.
//inputType: What kind of movement this joint should be controlled by, such as speed or position input.
//alg: the IOAlgorithm this joint should use
//cont1: The first output device controlling the first motor on this joint
//cont2: The second output device controlling the second motor on this joint
//feed: A pointer to the feedback device used on this joint.
RotateJoint::RotateJoint(ValueType inputType, IOAlgorithm *alg, OutputDevice* cont1, OutputDevice* cont2, FeedbackDevice* feed) : JointInterface()
{
	//assignments
	inType = inputType;
	controller1 = cont1;
	controller2 = cont2;
  coupled = false;
  motorOneVirtualPower = 0;
  motorTwoVirtualPower = 0;
  algorithmUsed = true;

	feedback = feed;
	manip = alg;
  manip -> setFeedDevice(feed);

  //checks to make sure the passed arguments all work with each other, that is that the algorithm's input type is the same as what the user is putting in, and
  //that the algorithm's output value type is what the output device expects to take in, etc
  if((inputType == alg->inType) && (cont1->inType == alg->outType) && (alg->feedbackInType == feed->fType) && (cont1->inType == cont2->inType))
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

RotateJoint::~RotateJoint()
{
  //deliberately left blank; we don't want to destroy the pointers to other classes as the main program could still be handling them
}

//run the output algorithm for this tilt joint correctly (I mean, hopefully).
//calls the algorithm to manipulate the input and sends it to each controller.
//One device gets an inverted direction from the other, since they move in opposite tandem on rotate joints.
//input: a long that represents the desired movement. What values this int is constrained to depend on what this joint was set up to take as an input.
//For example, if this joint runs off of speed input then the values are constrained between SPEED_MIN and SPEED_MAX, and otherwise for the similar
//ranges defined in the framework's .h file
//returns: The status of attempting to control this joint. Such as if the output is now running, or if it's complete, or if there was an error
JointControlStatus RotateJoint::runOutputControl(const long movement)
{
  long mov; //var used as interrum value since algorithm can change the value
  bool motionComplete;
  JointControlStatus returnStatus;

  if(!enabled)
  {
    returnStatus = DeviceDisabled;
  }
  else if(verifyInput(movement) == false)
  {
    returnStatus = InvalidInput;
  }
  else if(validConstruction)
  {

    //runs the algorithm on the input. If there is none, output is just passed directly to output device
    if(algorithmUsed)
    {
      mov = manip->runAlgorithm(movement, &motionComplete);
    }
    else
    {
      mov = movement;
      motionComplete = true;
    }
    
    //if motionComplete returned false but movement is 0, that's an indication that an error state occured
    if(motionComplete == false && mov == 0)
    {
      returnStatus = InvalidInput;
    }
    
    else if(motionComplete == true)
    {
      returnStatus = OutputComplete;
    }
    else
    {
      returnStatus = OutputRunning;
    }

    motorOneVirtualPower = -mov;
    motorTwoVirtualPower = mov;
    
    int motorOneTruePower = motorOneVirtualPower;
    int motorTwoTruePower = motorTwoVirtualPower;

    //this only happens if this joint has been coupled with another joint
    //the coupled logic will modify the power calculated by the algorithm
    //to allow smooth tilting and rotating at the same time.
    //It is automatically assumed that the other joint is a tilt joint
    if(coupled)
    {
      motorOneTruePower += coupledJoint->motorOneVirtualPower;
      if (motorOneTruePower > POWER_MAX)
      {
        motorOneTruePower = POWER_MAX;
      }
      else if (motorOneTruePower < POWER_MIN)
      {
        motorOneTruePower = POWER_MIN;
      }

      motorTwoTruePower += coupledJoint->motorTwoVirtualPower;
      if (motorTwoTruePower > POWER_MAX)
      {
        motorTwoTruePower = POWER_MAX;
      }
      else if (motorTwoTruePower < POWER_MIN)
      {
        motorTwoTruePower = POWER_MIN;
      }
    }

    //send to the motor move command
    controller1->move(motorOneTruePower);

    //send command to motor 2. Since this is a rotate joint, the motors need to go in opposite directions so send the second one a negafied value
    controller2->move(motorTwoTruePower);
  }

  //if this joint wasn't properly constructed, nothing is run
  else
  {
    returnStatus = InvalidConstruction;
  }

  return(returnStatus);
}

void RotateJoint::stop()
{
  controller1->stop();
  controller2->stop();
  motorOneVirtualPower = 0;
  motorTwoVirtualPower = 0;
}

void RotateJoint::disableJoint()
{
  stop();
  //since disabling controllers would affect both joints if this is a differential joint,
  //don't turn off the devices. Just stop the joint until enable is called
  if(!coupled)
  {
    controller1->setPower(false);
    controller2->setPower(false);
  }
  enabled = false;
}

void RotateJoint::enableJoint()
{
  //since enabling controllers would affect both joints if this is a differential joint,
  //don't turn on the devices. Just enable this joint to move
  if(!coupled)
  {
    controller1->setPower(true);
    controller2->setPower(true);
  }
  enabled = true;
}

void RotateJoint::setDifferentialMode(TiltJoint* otherJoint)
{
  this->coupledJoint = otherJoint;
  this->coupled = true;
  otherJoint->coupledJoint = this;
  otherJoint->coupled = true;
}

