#ifndef ABSTRACTFRAMEWORK_H_
#define ABSTRACTFRAMEWORK_H_

#include "RoveJointUtilities.h"

class JointInterface;
class IOAlgorithm;
class OutputDevice;
class FeedbackDevice;

class TiltJoint;
class SingleMotorJoint;
class RotateJoint;

class JointInterface
{
  protected:

    //expected input type from base station
    ValueType inType;

    //algorithm is either passed in, or created dymanically based on what is passed to the control framework interface.
    //called in runOutputControl
    IOAlgorithm* manip;

    //Passed to the framework interface if feedback is used. Used when looping in certain algorithms
    FeedbackDevice* feedback;

    //pointer to the output device instance which is passed in when creating the framework interface
    //called in runOutputControl
    OutputDevice* controller1;

    //tracks whether or not the parameters passed via construction were valid
    bool validConstruction;
    
    //tracks whether or not the user has passed in an algorithm to use; if not, commands are passed directly to output
    bool algorithmUsed;
    
    //tracks whether or not the joint has been enabled for operation
    bool enabled;

    //function that checks to see if the user put in a proper input value when calling the runOutputControl function
    //returns true if the input is in a valid range, false if it's not
    bool verifyInput(long inputToVerify);

    JointInterface()
    {
      enabled = true;
    };

  public:

    //Runs the output control for this joint, IE making it move or checking
    //feedback to see if it needs to move, whatever the algorithm for
    //this joint is deemed to be, it runs it.
    //Must pass an integer for this implementation. Will break otherwise.
    //returns: The status of attempting to control this joint. Such as if the output is now running, or if it's complete, or if there was an error
    virtual JointControlStatus runOutputControl(const long movement) = 0;
    
    //replaces the current joint's closed loop algorithm component with a different one
    //input: the new type of input the joint is going to be given when runOutputControl is called
    //       the new algorithm module and its corresponding feedback device
    //returns: true if swap was successful, false if not and previous settings retained
    //warning: not thread safe
    bool switchToClosedAlg(ValueType newInputType, IOAlgorithm* newAlgorithm, FeedbackDevice* newFeed);
	
    //tells joint to switch to open loop control
    //input: the new type of input the joint is going to be given when runOutputControl is called
    //returns: true if swap was successful, false if not and previous settings retained
    //warning: not thread safe
    bool switchToOpenAlg(ValueType newInputType);
    
    //tells the joint to halt. Note that this won't keep the joint from moving if called again; 
    //use disable for that
    virtual void stop() = 0;
    
    //turns the joint off; it will stop moving until enabled
    virtual void disableJoint() = 0;
    
    //turns the joint on after being disabled
    virtual void enableJoint() = 0;
};

class FeedbackDevice
{
	friend class SingleMotorJoint;
	friend class TiltJoint;
	friend class RotateJoint;
  friend class JointInterface;

	public:

		//returns feedback. Public because all the IOAlgorithm classes need to be able to call it,
		//and friend isn't inherited so we can't just make the abstract class our friend. Also it's useful for the user if they 
    //want to see the feedback
		//returns: a value representing feedback, the range of the value depends on what input type this device returns.
		//For example, if this device returns speed feedback, the values shall be in the range between SPEED_MIN and SPEED_MAX
		virtual long getFeedback() = 0;

		ValueType fType;
};

class OutputDevice
{
	//Joint interface needs access to its functions
	friend class SingleMotorJoint;
	friend class TiltJoint;
	friend class RotateJoint;
	friend class JointInterface;

  public:
    
    //gets the current moving value of the device. 
    //return value depends on what value type the device operates on; a speed value for speed devices, etc.
    virtual long getCurrentMove() = 0;

	protected:

		//calls the device to move the motor, based on the passed value of movement.
		//Input: Can be values based on any of the input types as defined in the enum above, and the ranges for these values
		//should stay within the max and min constants for each type
		virtual void move(const long movement) =  0;

		//expected input that the output device wants.
		ValueType inType;

		//used for if the specific controller is mounted backwards on the motor joint.
		//True means invert the signal (backwards) False means just send the signal
		bool invert;

    //tracks whether or not the device is enabled/powered on by the joint manager
    bool enabled;
    
    OutputDevice()
    {
      enabled = true;
    };
    
    //tells device to stop moving
    virtual void stop() = 0;
    
    //tells the device to power on or off.
    virtual void setPower(bool powerOn) = 0;
};

class IOAlgorithm
{
	friend class SingleMotorJoint;
	friend class TiltJoint;
	friend class RotateJoint;
  friend class JointInterface;

	protected:

    //pointer to the feedback device used on this joint, if there is any
    FeedbackDevice * feedbackDev;

    //the types of values that the algorithm takes in and gives out
    ValueType inType;
    ValueType outType;

    IOAlgorithm()
    {
      feedbackInitialized = false;
    };

    //the types of values that the algorithm expects to take in as feedback, if feedback is used
    ValueType feedbackInType;

    //if this IOAlgorithm uses a feedback device, this flag tracks whether or not the feedback device has been passed into the class yet
    bool feedbackInitialized;

		//run whatever algorithm this implements, returns value that can be directly passed to an output device
		//input: int in, representing the input values that need to be converted into output values.
		//The specific value constraints depend on the input and output types the algorithm implements; for example if an algorithm
		//is supposed to take in speed and output position, then its input is constrained by SPEED_MIN and SPEED_MAX, and its output is constrained by POS_MIN and POS_MAX
    //bool * ret_outputFinished: parameter passed by pointer, returns true if the joint has finished its controlled movement and ready to exit the control loop,
    //false if it's still in the middle of getting to its desired finished state IE in the middle of moving to a desired end position or reaching a desired end speed
		virtual long runAlgorithm(const long input, bool * ret_OutputFinished) = 0;

    //if this IOAlgorithm uses feedback device, this function is used by the joint interface to set it, and sets the feedbackInitialized flag to true
    void setFeedDevice(FeedbackDevice *fdDev);
};

#endif
