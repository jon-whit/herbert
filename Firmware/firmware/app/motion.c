/////////////////////////////////////////////////////////////
//
//  motion.c
//
//  High-level instrument motion control for door, lid and filters

#include <limits.h>
#include <assert.h>
#include <timer.h>
#include <steppers.h>
#include <motion.h>
#include <string.h>
#include <stdio.h>
#include <comm.h>
#include <os.h>
#include <flash.h>
#include <relay.h>
#include <stepper_hw.h>



///////////////////////////////////////////////////
// Constants

#define BETA_HARDWARE_SUPPORT 1 //TODO: Remove when moving to production hardware


#define ERROR_DESC_SIZE 200

#define MAX_ROTATIONS 100


enum MotorConstants
{
    invalid_position      = INT_MAX,
    initEnableWaitTime_ms = 2000,
};


enum FilterConstants
{
    filterInitializationTimeout_ms = 4000,
    filterMicrosteps               = 16,
    filterStepsPerRevolution       = (int)(filterMicrosteps * 360 / 7.5),    // The filter stepper motor resolution is 7.5 degrees per full step.
    minFilterSeparation            = 100,
    numFilterPins                  = 7,

    maxSpaceFindFilterSteps        = 50,
    spaceFindStepSize              = 5,

    filterPinSearchFrequency       = 25,
    filterPinSearchRampSteps       = defaultRampSteps,
};


enum DoorConstants
{
    door_initialization_frequency     = 800,
    door_initialization_ramp_steps    = 0,
    door_initialization_timeout_ms    = 15000,
    door_preinitialization_move_steps = 500,
    door_min_closed_position          = 10000,
    door_max_closed_position          = 12000,
    door_default_closed_position      = 11100,
    door_open_position                = 0,
    door_pin_position_threashold      = 10,
    door_max_home_sensor_steps        = 300, //TODO: Tighten up this number for pre-production
    door_max_alt_sensor_steps         = 700, //TODO: Tighten up this number for pre-production
};


enum LidConstants
{
    lid_initialization_frequency     = defaultSlowFrequency,
    lid_initialization_ramp_steps    = 100,
    lid_initialization_timeout_ms    = 25000,
    lid_preinitialization_move_steps = 500,
    lid_preinit_door_move_steps      = 1000,
    lid_preinit_door_steps_past_alt  = 200,
    lid_lowered_position             = 14500,
    lid_min_compressed_position      = 16000,
    lid_max_compressed_position      = 19000,
    lid_default_compressed_position  = 17000,
    lid_raised_position              = 0,
    lid_pin_position_threashold      = 10,
    lid_max_home_sensor_steps        = 200,
    lid_max_alt_sensor_steps         = 2300,
};

enum ArmMotionConstants
{
    halfTurnSteps                    = 90,
    quarterTurnStepsClockwise        = 40,
    quarterTurnStepsCounterClockwise = -40,

    armMotionTimeout_ms              = 100000,
    actuateTimeout_ms                = 500,
};

///////////////////////////////////////////////////
// Local types and macros

typedef enum
{
    state_idle,
    state_initializing,
    state_error,
    state_aborting,
    state_actuatingArmIn,
    state_armActuatedIn,
    state_armSpinning,
    state_armDoneSpinning,
    state_actuatingArmOut,
} MotionControlState;

typedef enum
{
    initState_idle,
    initState_filterStart,
    initState_filterFindSpace,
    initState_filterRun,
    initState_filterEnd,
    initState_lidStart,
    initState_lidRun1,
    initState_lidRun2,
    initState_lidEnd,
    initState_doorStart,
    initState_doorRun,
    initState_doorEnd,
    initState_completeNotification,
} InitStates;


typedef enum
{
    lidLoweringState_lowering,
    lidLoweringState_compressing,
} LidLoweringState;



typedef struct
{
    Timer      timer;
    InitStates state;
} InitData;

typedef struct
{
    bool initialized;
    int  findSpaceSteps;
    int  filterPositions[num_filters];
    int  sensorBreakPositions[filterStepsPerRevolution];
    int  positionIndex;
    int  slowFrequency;
    int  fastFrequency;
    int  rampSteps;
} FilterData;

typedef struct
{
    bool initialized;
    bool disabled;
    int  closedPosition;
    int  startPosition;
    bool homePinFound;
    bool altPinFound;
    int  homePinFoundPos;
    int  altPinFoundPos;
    int  slowFrequency;
    int  fastFrequency;
    int  rampSteps;
} DoorData;

typedef struct
{
    bool             initialized;
    LidLoweringState loweringState;
    int              compressedPosition;
    int              startPosition;
    bool             homePinFound;
    int              altPinCount;
    int              altSpaceCount;
    int              homePinMakePos;
    int              altPinMake1Pos;
    int              altPinBreak1Pos;
    int              altPinMake2Pos;
    int              altPinBreak2Pos;
    PlateType        plateType;
    int              slowFrequency;
    int              fastFrequency;
    int              rampSteps;
} LidData;


typedef struct
{
    OfflineTaskCompleteCallback function;
    int                         reference;
} OperationCallback;

typedef struct 
{
    StepperMotor      motor;
    TurnSize          turnSize;
    RotationDirection direction;
    Timer             timer;
    Timer             actuateTimer;
} ArmMotionData;

///////////////////////////////////////////////////
// Local function prototypes

void motionTest();

static void finishMotorMove(ErrorCodes error, const char* errorDesc);
static char convertMotorToChar(StepperMotor motor);


///////////////////////////////////////////////////
// Local data

static struct MotionData
{
    InitData           init;
    FilterData         filter;
    DoorData           door;
    LidData            lid;

    MotionControlState state;

    OperationCallback  callback;
    
    ErrorCodes         error;
    char               errorDesc[ERROR_DESC_SIZE];
} motionData;

static struct QueuedRotations
{
    int                totalRotations;
    int                currentRotation;
    ArmMotionData      rotations[MAX_ROTATIONS];   
} queuedRotations;

static bool isSensorAdjustmentDone = false;


///////
////////////////////////////////////////////
// Interface functions

void motionInit()
{
    memset(&motionData, 0, sizeof(motionData));

    motionData.state = state_idle;
    
    printf("\nMotion Init...\n");

    motionRestoreDefaults();

    // Low-level Init
    stepper_init();

    memset(&queuedRotations, 0, sizeof(queuedRotations));
    queuedRotations.totalRotations = 0;
    queuedRotations.currentRotation = 0;
}


void motionRestoreDefaults(void)
{
    motionData.filter.slowFrequency = defaultSlowFrequency;
    motionData.filter.fastFrequency = defaultFastFrequency;
    motionData.filter.rampSteps     = defaultRampSteps;

    motionData.door.slowFrequency   = defaultSlowFrequency;
    motionData.door.fastFrequency   = defaultFastFrequency;
    motionData.door.rampSteps       = defaultRampSteps;

    motionData.lid.slowFrequency    = defaultSlowFrequency;
    motionData.lid.fastFrequency    = defaultFastFrequency;
    motionData.lid.rampSteps        = defaultRampSteps;
}

void motionAbort()
{
    stepper_abort();
    motionData.state = state_aborting;
}

bool motionProcess(void* unused)
{
    // Handle special state cases first
    if(motionData.state == state_idle)
    {
        motionData.callback.function = NULL;
    }
    else if(motionData.state == state_initializing)
    {
        motionData.state = state_idle;
    }
    // Checks to see if stepper is rotating, not exactly sure why this is important
    else if(!stepper_busy())
    {
        switch(motionData.state)
        {
        case state_error:
            finishMotorMove(motionData.error, motionData.errorDesc);
            break;

        case state_aborting:
            motionData.state = state_idle;
            break;

        case state_actuatingArmIn:
	    // TODO: Check for the switches and make sure it actually acuated in before changing state
	    // TODO: Either have a queue system 
	    if(timerExpired(&queuedRotations.rotations[queuedRotations.currentRotation].actuateTimer))
            {
                motionData.state = state_armActuatedIn;
            }
            break;

        case state_armActuatedIn:
            switch(queuedRotations.rotations[queuedRotations.currentRotation].turnSize)
            {
                case turn_half:
                    stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, halfTurnSteps);
                    break;

                case turn_quarter:
                    switch(queuedRotations.rotations[queuedRotations.currentRotation].direction)
                    {
                        case rotation_clockwise:
                            stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, quarterTurnStepsClockwise);
                            break;

                        case rotation_counterClockwise:
                            stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, quarterTurnStepsCounterClockwise);
                            break;
                    }    
                    break;  
            }
            motionData.state = state_armSpinning;
            isSensorAdjustmentDone = false;
            break;

        case state_armSpinning:
	    // Makes sure that the stepper is finished moving
            if(!stepper_busy())
            {
                if(isSensorBeamBroken(queuedRotations.rotations[queuedRotations.currentRotation].motor))
                {
                    if(queuedRotations.rotations[queuedRotations.currentRotation].direction == rotation_clockwise)
                    {
                        stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, 1);
                    }
                    else
                    {
                        stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, -1);
                    }
                }
                else if(!isSensorAdjustmentDone)
                {
                    if(queuedRotations.rotations[queuedRotations.currentRotation].direction == rotation_clockwise)
                    {
                        stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, 2);
                    }
                    else
                    {
                        stepper_move_relative(queuedRotations.rotations[queuedRotations.currentRotation].motor, -2);
                    }
                    isSensorAdjustmentDone = true;
                }
                else 
                {
                    motionData.state = state_armDoneSpinning;
                    isSensorAdjustmentDone = false;
                }
            }
            
            break;

        case state_armDoneSpinning:
            ActuateArmsOut();
            startTimer(&queuedRotations.rotations[queuedRotations.currentRotation].actuateTimer, MSEC_TO_TICKS(actuateTimeout_ms));
            motionData.state = state_actuatingArmOut;
            break;            

        case state_actuatingArmOut:
            if(timerExpired(&queuedRotations.rotations[queuedRotations.currentRotation].actuateTimer))
            {
                queuedRotations.currentRotation++;
                if(queuedRotations.currentRotation < queuedRotations.totalRotations)
                {            
                        switch(queuedRotations.rotations[queuedRotations.currentRotation].motor)
                        {
                            case stepperU:
                                ActuateArmIn('U');
                                break;

                            case stepperF:
                                ActuateArmIn('F');
                                break;

                            case stepperR:
                                ActuateArmIn('R');
                                break;

                            case stepperD:
                                ActuateArmIn('D');
                                break;

                            case stepperB:
                                ActuateArmIn('B');
                                break;

                            case stepperL:
                                ActuateArmIn('L');
                                break;

                            default:
                                break;
                        }
                        startTimer(&queuedRotations.rotations[queuedRotations.currentRotation].actuateTimer, MSEC_TO_TICKS(actuateTimeout_ms));
                        motionData.state = state_actuatingArmIn;
                }
                else
                {
		    printf("%d\n", (int)getTimer_ms(&queuedRotations.rotations[queuedRotations.currentRotation].timer));
		    disableMotors();
		    queuedRotations.currentRotation = 0;
		    queuedRotations.totalRotations = 0;
                    motionData.state = state_idle;
                }
            }
            break;

        default:
            ASSERT(false);
        }
    }

    return true;
}

///////////////////////////////////////////////////
// Local filter functions
void queueRotation(StepperMotor stepperMotor, RotationDirection direction, TurnSize turnSize)
{
    queuedRotations.rotations[queuedRotations.totalRotations].motor        = stepperMotor;
    queuedRotations.rotations[queuedRotations.totalRotations].direction    = direction;
    queuedRotations.rotations[queuedRotations.totalRotations].turnSize     = turnSize;
    queuedRotations.totalRotations++;
}

void executeRotations(void)
{
    queuedRotations.currentRotation = 0;
    ASSERT(queuedRotations.totalRotations > 0); 
    // TODO: ASSERT that the switches etc etc
    startTimer(&queuedRotations.rotations[0].timer, MSEC_TO_TICKS(actuateTimeout_ms));
    startTimer(&queuedRotations.rotations[0].actuateTimer, MSEC_TO_TICKS(actuateTimeout_ms));

    switch(queuedRotations.rotations[0].motor)
    {
        case stepperU:
            ActuateArmIn('U');
            break;

        case stepperF:
            ActuateArmIn('F');
            break;

        case stepperR:
            ActuateArmIn('R');
            break;

        case stepperD:
            ActuateArmIn('D');
            break;

        case stepperB:
            ActuateArmIn('B');
            break;

        case stepperL:
            ActuateArmIn('L');
            break;

        default:
            break;
    }

    motionData.state = state_actuatingArmIn;
}

bool isIdle(void) 
{
    return motionData.state == state_idle;
}


///////////////////////////////////////////////////
// Debug functions

int getStepperParameter(StepperMotor motor, StepperParameter stepperParam)
{
    switch(motor)
    {
        case stepperU:
            switch(stepperParam)
            {
                case stepperFastFrequency: return motionData.lid.fastFrequency;
                case stepperSlowFrequency: return motionData.lid.slowFrequency;
                case stepperRampSteps:     return motionData.lid.rampSteps;
                default: break;
            }
        case stepperF:
            switch(stepperParam)
            {
                case stepperFastFrequency: return motionData.door.fastFrequency;
                case stepperSlowFrequency: return motionData.door.slowFrequency;
                case stepperRampSteps:     return motionData.door.rampSteps;
                default: break;
            }
        case stepperR:
            switch(stepperParam)
            {
                case stepperFastFrequency: return motionData.filter.fastFrequency;
                case stepperSlowFrequency: return motionData.filter.slowFrequency;
                case stepperRampSteps:     return motionData.filter.rampSteps;
                default: break;
            }
        default: break;
    }

    ASSERT(false);
    return 0;
}

void setStepperParameter(StepperMotor motor, StepperParameter stepperParam, int value)
{
    stepper_set_parameter(motor, stepperParam, value);
    switch(motor)
    {
        case stepperU:
            switch(stepperParam)
            {
                case stepperFastFrequency: motionData.lid.fastFrequency = value; return;
                case stepperSlowFrequency: motionData.lid.slowFrequency = value; return;
                case stepperRampSteps:     motionData.lid.rampSteps     = value; return;
                default: break;
            }
        case stepperF:
            switch(stepperParam)
            {
                case stepperFastFrequency: motionData.door.fastFrequency = value; return;
                case stepperSlowFrequency: motionData.door.slowFrequency = value; return;
                case stepperRampSteps:     motionData.door.rampSteps     = value; return;
                default: break;
            }
        case stepperR:
            switch(stepperParam)
            {
                case stepperFastFrequency: motionData.filter.fastFrequency = value; return;
                case stepperSlowFrequency: motionData.filter.slowFrequency = value; return;
                case stepperRampSteps:     motionData.filter.rampSteps     = value; return;
                default: break;
            }
        default: break;
    }

    ASSERT(false);
}

static void finishMotorMove(ErrorCodes error, const char* errorDesc)
{
    motionData.state = state_idle;

    if(motionData.callback.function)
    {
        motionData.callback.function(motionData.callback.reference, error, errorDesc);
        motionData.callback.function = NULL;
    }

    printf("Motor move finished.\n");
}

static char convertMotorToChar(StepperMotor motor)
{
    switch(motor)
    {
        case stepperU: return 'U';
        case stepperF: return 'F';
        case stepperR: return 'R';
        case stepperD: return 'D';
        case stepperB: return 'B';
        case stepperL: return 'L';
        default: return 0;
    }
}


// EOF
