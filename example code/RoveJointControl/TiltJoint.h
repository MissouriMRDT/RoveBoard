#ifndef TILTJOINT_H_
#define TILTJOINT_H_

#include "AbstractFramework.h"
#include "RoveJointUtilities.h"

class TiltJoint : public JointInterface
{
	protected:

		OutputDevice* controller2;
    
    //the other joint with which this joint is coupled with
    RotateJoint* coupledJoint;

    //tracks whether or not the joint is coupled with another joint, making this entire thing a differential joint mechanically
    //(ask a mechanical if you don't know what that is)
    bool coupled;
    
	public:

    //variables to store the power of either motor; useful if this is a differential joint, as the movements of one
        //affect the other so virtual power variables are used to track what this specific part of the joint is trying
        //to move at, to compare with the tilt joint this is coupled with
    int motorOneVirtualPower;
    int motorTwoVirtualPower;
    
		//creates the joint interface for a tilt joint with a feedback device.
		//Note both output devices are assumed to have the same input type
		//inputType: What kind of movement this joint should be controlled by, such as speed or position input.
		//alg: the closed loop IOAlgorithm used by this joint
		//cont1: The first output device controlling the first motor on this joint
		//cont2: The second output device controlling the second motor on this joint
		//feed: The feedback device used with this joint
		TiltJoint(ValueType inputType, IOAlgorithm *alg, OutputDevice* cont1, OutputDevice* cont2, FeedbackDevice* feed);

		//creates joint interface for a tilt joint with no feedback.
		//Constructor automatically chooses an open loop algorithm that inputs the specified inputType and outputs the values the output device accepts
		//Note both output devices are assumed to have the same input type
		//inputType: What kind of movement this joint should be controlled by, such as speed or position input.
		//cont1: The first output device controlling the first motor on this joint
		//cont2: The second output device controlling the second motor on this joint
		TiltJoint(ValueType inputType, OutputDevice* cont1, OutputDevice* cont2);

		~TiltJoint();

		//runs algorithm for moving two motors together so that it tilts the joint
		//calls the algorithm to manipulate the input and sends it to each controller.
		//Both devices get the same command since they're supposed to move together.
		//input: a long that represents the desired movement. What values this int is constrained to depends on what this joint was set up to take as an input.
		//For example, if this joint runs off of speed input then the values are constrained between SPEED_MIN and SPEED_MAX, and otherwise for the similar
		//ranges defined in the framework's .h file
        //returns: The status of attempting to control this joint. Such as if the output is now running, or if it's complete, or if there was an error
		JointControlStatus runOutputControl(const long movement);
    
    //couples this differential joint with another tilt joint; differential joints 
    //are intertwined so need to interact with each other to properly gauge output power
    void setDifferentialMode(RotateJoint* otherJoint);
    
    //tells the joint to halt. Note that this won't keep the joint from moving if called again; 
    //use disable for that
    void stop();
    
    //turns the joint off; it will stop moving until enabled
    void disableJoint();
    
    //turns the joint on after being disabled
    void enableJoint();
    
    friend class RotateJoint;
};

#endif
