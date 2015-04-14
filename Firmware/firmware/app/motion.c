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



///////////////////////////////////////////////////
// Constants

#define BETA_HARDWARE_SUPPORT 1 //TODO: Remove when moving to production hardware


#define ERROR_DESC_SIZE 200


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
    halfTurnSteps                    = 200,
    quarterTurnStepsClockwise        = 100,
    quarterTurnStepsCounterClockwise = -100,

    armMotionTimeout_ms              = 100000,
    actuateTimeout_ms                = 1000,
};

///////////////////////////////////////////////////
// Local types and macros

typedef enum
{
    state_idle,
    state_initializing,
    state_settingFilter,
    state_openingDoor,
    state_closingDoor,
    state_raisingLid,
    state_loweringLid,
    state_openingCavityDoor,
    state_openingCavityLid,
    state_closingCavityDoor,
    state_closingCavityLid,
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



///////////////////////////////////////////////////
// Local function prototypes

void motionTest();

static bool verifyDoorPosition(int expectedPosition);
static bool verifyLidPosition(int expectedPosition);
static bool verifyDoorOpenedSensor();
static bool verifyDoorClosedSensor();
static bool verifyLidRaisedSensors();
static bool verifyLidLoweredSensors();

static void disableAndFinishMotorMove(StepperMotor stepper, ErrorCodes error, const char* errorDesc);
static void finishMotorMove(ErrorCodes error, const char* errorDesc);

static bool determineFilterPositions(int sensorBreakPositions[], int filterPositions[], char* errorDesc, unsigned errorDescSize);
static bool filterInitHomeSensorHook(int position);

static bool initProcess();
static bool notifyInitCompleteTask(void* unused);


static bool filterInitStart();
static bool filterInitRun();
static bool filterInitEnd();
static bool lidInitStart();
static bool lidInitEnd();
static bool lidInitRun1();
static bool lidInitRun2();
static bool startRaiseLid();
static bool startLowerLid();
static bool startCompressLid();
static bool lidHomeSensorMakeCallback(int position);
static bool lidAltSensorMakeCallback(int position);
static bool lidAltSensorBreakCallback(int position);
static bool lidInitDoorAltSensorMakeCallback(int position);
static bool doorInitStart();
static bool doorInitEnd();
static bool doorInitRun();
static bool startOpenDoor();
static bool startCloseDoor();
static bool doorOpenHomeSensorCallback(int position);
static bool doorCloseAltSensorMakeCallback(int position);




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

static struct ArmMotionData
{
    StepperMotor      motor;
    TurnSize          turnSize;
    RotationDirection direction;
    int               totalRotations;
    int               currentRotation;
    Timer             timer;
    Timer             actuateTimer;
} armMotionData;


///////////////////////////////////////////////////
// Interface functions

void motionInit()
{
    memset(&motionData, 0, sizeof(motionData));

    motionData.state = state_idle;


    printf("\nMotion Init...\n");

    motionRestoreDefaults();


    // Filter Init
    motionData.filter.initialized   = false;



    // Door Init
    int doorClosedPosition;

    if(doorClosedPosition < door_min_closed_position ||
       doorClosedPosition > door_max_closed_position)
    {
        sendErrorMsg( err_deviceFault, "Error reading door closed position!" );
        doorClosedPosition = door_default_closed_position;
    }

    motionData.door.initialized    = false;
    motionData.door.disabled       = false;
    motionData.door.closedPosition = doorClosedPosition;



    // Lid Init
    int lidCompressedPosition;

    if(lidCompressedPosition < lid_min_compressed_position ||
       lidCompressedPosition > lid_max_compressed_position)
    {
        sendErrorMsg( err_deviceFault, "Error reading lid PCR position!" );
        lidCompressedPosition = lid_default_compressed_position;
    }

    motionData.lid.initialized        = false;
    motionData.lid.compressedPosition = lidCompressedPosition;
    motionData.lid.plateType          = plateType_unknown;


    // Low-level Init
    stepper_init();
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



bool motionInitFilter(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state != state_idle)
    {
        callbackFunc(callbackRef, err_filterInitializationFailure, "Can't init filter - steppers busy");
        return false;
    }

    motionData.callback.function  = callbackFunc;
    motionData.callback.reference = callbackRef;
    motionData.state = state_initializing;

    return filterInitStart();
}



bool motionInitLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state != state_idle)
    {
        callbackFunc(callbackRef, err_lidInitializationFailure, "Can't init lid - steppers busy");
        return false;
    }

    motionData.callback.function  = callbackFunc;
    motionData.callback.reference = callbackRef;
    motionData.state = state_initializing;
    lidInitStart();

    return true;
}



bool motionInitDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state != state_idle)
    {
        callbackFunc(callbackRef, err_doorInitializationFailure, "Can't init door - steppers busy");
        return false;
    }

    motionData.callback.function  = callbackFunc;
    motionData.callback.reference = callbackRef;
    motionData.state = state_initializing;
    return doorInitStart();
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
        if(initProcess())
        {
            motionData.state = state_idle;
        }
    }
    else if(!stepper_busy())
    {
        switch(motionData.state)
        {
        case state_settingFilter:
            finishMotorMove(err_noError, NULL);
            break;

        case state_closingDoor:
        case state_closingCavityDoor:
            // Fall through - Handle these states together
            if(!verifyDoorPosition(motionData.door.closedPosition))
            {
                disableAndFinishMotorMove(stepperF, err_doorCloseFailure, "Door not closed");
            }
            else if(!verifyDoorClosedSensor())
            {
                disableAndFinishMotorMove(stepperF, err_doorCloseFailure, "Door closed sensor failed");
            }
            else if(stepper_is_at_home_position(stepperF))
            {
                disableAndFinishMotorMove(stepperF, err_doorCloseFailure, "Door closed but open sensor also triggered");
            }
            else
            {
                if(motionData.state == state_closingDoor)
                {
                    finishMotorMove(err_noError, NULL);
                }
                else
                {
                    if(startLowerLid())
                    {
                        motionData.state = state_closingCavityLid;
                    }
                    else
                    {
                        finishMotorMove(err_lidCannotMoveStepperBusy, "Stepper system busy");
                    }
                }
            }
            break;

        case state_openingDoor:
        case state_openingCavityDoor:
            // Fall through - Handle these states the same
            if(!verifyDoorOpenedSensor())
            {
                disableAndFinishMotorMove(stepperF, err_doorOpenFailure, "Door open sensor failed");
            }
            else if(stepper_is_at_alt_position(stepperF))
            {
                disableAndFinishMotorMove(stepperF, err_doorOpenFailure, "Door open but closed sensor also triggered");
            }
            else
            {
                finishMotorMove(err_noError, NULL);
            }
            break;

        case state_raisingLid:
            if(!verifyLidRaisedSensors())
            {
                finishMotorMove(err_lidRaiseFailure, "Lid raise not detected");
            }
            else
            {
                finishMotorMove(err_noError, NULL);
            }
            break;

        case state_loweringLid:
        case state_closingCavityLid:
            // Fall through - Handle these states the same
            if(motionData.lid.loweringState == lidLoweringState_lowering)
            {
                if(!startCompressLid())
                {
                    finishMotorMove(err_lidLowerFailure, "Compress lid failed");
                }
            }
            else
            {
                if(!verifyLidLoweredSensors())
                {
                    finishMotorMove(err_lidLowerFailure, "Lid lower not detected");
                }
                else
                {
                    finishMotorMove(err_noError, getPlateTypeName());
                }
            }
            break;

        case state_openingCavityLid:
            if(!verifyLidPosition(lid_raised_position))
            {
                finishMotorMove(err_lidRaiseFailure, "Lid not raised");
            }
            else if(!verifyLidRaisedSensors())
            {
                finishMotorMove(err_lidRaiseFailure, "Lid raised sensor failed");
            }
            else if(startOpenDoor())
            {
                motionData.state = state_openingCavityDoor;
            }
            else
            {
                finishMotorMove(err_doorCannotMoveStepperBusy, "Stepper system busy");
            }
            break;

        case state_error:
            finishMotorMove(motionData.error, motionData.errorDesc);
            break;

        case state_aborting:
            motionData.state = state_idle;
            break;

        case state_actuatingArmIn:
            if(timerExpired(&armMotionData.actuateTimer))
            {
                motionData.state = state_armActuatedIn;
            }
            break;

        case state_armActuatedIn:
            switch(armMotionData.turnSize)
            {
                case turn_half:
                    stepper_move_relative(armMotionData.motor, halfTurnSteps);
                    break;

                case turn_quarter:
                    switch(armMotionData.direction)
                    {
                        case rotation_clockwise:
                            stepper_move_relative(armMotionData.motor, quarterTurnStepsClockwise);
                            break;

                        case rotation_counterClockwise:
                            stepper_move_relative(armMotionData.motor, quarterTurnStepsCounterClockwise);
                            break;
                    }    
                    break;  
            }
            motionData.state = state_armSpinning;
            break;

        case state_armSpinning:
            if(!stepper_busy())
                motionData.state = state_armDoneSpinning;
            break;

        case state_armDoneSpinning:
            ActuateArmsOut();
            startTimer(&armMotionData.actuateTimer, MSEC_TO_TICKS(actuateTimeout_ms));
            motionData.state = state_actuatingArmOut;
            break;            

        case state_actuatingArmOut:
            if(timerExpired(&armMotionData.actuateTimer))
            {
                if(armMotionData.currentRotation < armMotionData.totalRotations)
                {            
                        switch(armMotionData.motor)
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

                            default:
                                break;
                        }

                        motionData.state = state_actuatingArmIn;
                        armMotionData.currentRotation++;
                }
                else
                {
                    printf("%d\n", (int)getTimer_ms(&armMotionData.timer));
                    disableMotors();
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



Filters getFilterPosition()
{
    if(!motionData.filter.initialized)
    {
        return filter_unknownPosition;
    }

    if(motionData.state == state_settingFilter)
    {
        return filter_moving;
    }

    int pos = get_stepper_position(stepperR);

    int filter;
    for(filter = 0; filter < num_filters; filter++)
    {
        if(pos == motionData.filter.filterPositions[filter])
        {
            return (Filters)filter;
        }
    }

    return filter_unknownPosition;
}



void setFilter(Filters filter, OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(filter < (Filters)num_filters);
    ASSERT(callbackFunc);

    if(!motionData.filter.initialized || motionData.filter.filterPositions[filter] == invalid_position)
    {
        char errorDesc[ERROR_DESC_SIZE];

        snprintf(errorDesc, ERROR_DESC_SIZE, "Filter can't move.  Filter is not initialized.");
        callbackFunc(callbackRef, err_filterNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    stepper_set_parameter(stepperR, stepperSlowFrequency, motionData.filter.slowFrequency);
    stepper_set_parameter(stepperR, stepperFastFrequency, motionData.filter.fastFrequency);
    stepper_set_parameter(stepperR, stepperRampSteps,  motionData.filter.rampSteps);

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(stepper_move_to_position(stepperR, motionData.filter.filterPositions[filter]))
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_settingFilter;
        printf("Moving Filter...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_filterCannotMoveStepperBusy, "Can't move filter.");
    }
}



DoorPosition getDoorPosition()
{
    if(!motionData.door.initialized)
    {
        return door_unknownPosition;
    }

    if(motionData.state == state_openingDoor ||
       motionData.state == state_openingCavityDoor)
    {
        return door_opening;
    }

    if(motionData.state == state_closingDoor ||
       motionData.state == state_closingCavityDoor)
    {
        return door_closing;
    }

    if(get_stepper_position(stepperF) == motionData.door.closedPosition)
    {
        return door_closed;
    }

    if(get_stepper_position(stepperF) == door_open_position)
    {
        return door_opened;
    }

    return door_unknownPosition;
}



void openDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.door.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Door can't open.  Door not initialized.");
        callbackFunc(callbackRef, err_doorNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Door can't open.  Lid is not initialized.");
        callbackFunc(callbackRef, err_doorCannotMoveLidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(get_stepper_position(stepperU) != lid_raised_position ||
       !stepper_is_at_home_position(stepperU))
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Door can't open.  Lid is not raised (pos = %d, home sensor = %d)",
                 get_stepper_position(stepperU), stepper_is_at_home_position(stepperU));
        callbackFunc(callbackRef, err_doorCannotMoveLidNotRaised, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle && startOpenDoor())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_openingDoor;
        printf("Opening Door...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_doorCannotMoveStepperBusy, "Can't open door.");
    }
}



void closeDoor(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.door.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Door can't close.  Door not initialized.");
        callbackFunc(callbackRef, err_doorNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Door can't close.  Lid is not initialized.");
        callbackFunc(callbackRef, err_doorCannotMoveLidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle && startCloseDoor())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_closingDoor;
        printf("Closing Door...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_doorCannotMoveStepperBusy, "Can't close door.");
    }
}




LidPosition  getLidPosition()
{
    if(!motionData.lid.initialized)
    {
        return lid_unknownPosition;
    }

    if(motionData.state == state_raisingLid ||
       motionData.state == state_openingCavityLid)
    {
        return lid_raising;
    }

    if(motionData.state == state_loweringLid ||
       motionData.state == state_closingCavityLid)
    {
        return lid_lowering;
    }

    if(get_stepper_position(stepperU) == motionData.lid.compressedPosition)
    {
        return lid_lowered;
    }

    if(get_stepper_position(stepperU) == lid_raised_position)
    {
        return lid_raised;
    }

    return lid_unknownPosition;
}



void raiseLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't raise lid - not initialized.");
        callbackFunc(callbackRef, err_lidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.door.disabled)
    {
        if(!motionData.door.initialized)
        {
            snprintf(errorDesc, ERROR_DESC_SIZE, "Can't raise lid.  Door is not initialized.");
            callbackFunc(callbackRef, err_lidCannotMoveDoorNotInitialized, errorDesc);
            printf("%s\n", errorDesc);
            return;
        }

        if((get_stepper_position(stepperF) != motionData.door.closedPosition ||
            !stepper_is_at_alt_position(stepperF))                           &&
           get_stepper_position(stepperU)  != lid_raised_position)
        {
            snprintf(errorDesc, ERROR_DESC_SIZE, "Can't raise lid.  Door is not closed.");
            callbackFunc(callbackRef, err_lidCannotMoveDoorNotClosed, errorDesc);
            printf("%s\n", errorDesc);
            return;
        }
    }
    else
    {
        printf("Warning: moving Lid while Door is disabled\n");
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle && startRaiseLid())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_raisingLid;
        printf("Raising Lid...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_lidCannotMoveStepperBusy, "Can't raise lid.");
    }
}



void lowerLid(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't lower lid - not initialized.");
        callbackFunc(callbackRef, err_lidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.door.disabled)
    {
        if(!motionData.door.initialized)
        {
            snprintf(errorDesc, ERROR_DESC_SIZE, "Can't lower lid.  Door is not initialized.");
            callbackFunc(callbackRef, err_lidCannotMoveDoorNotInitialized, errorDesc);
            printf("%s\n", errorDesc);
            return;
        }

        if((get_stepper_position(stepperF) != motionData.door.closedPosition ||
            !stepper_is_at_alt_position(stepperF))                           &&
           get_stepper_position(stepperU) != motionData.lid.compressedPosition)
        {
            snprintf(errorDesc, ERROR_DESC_SIZE, "Can't lower lid.  Door is not closed.");
            callbackFunc(callbackRef, err_lidCannotMoveDoorNotClosed, errorDesc);
            printf("%s\n", errorDesc);
            return;
        }
    }
    else
    {
        printf("Warning: moving Lid while Door is disabled\n");
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle && startLowerLid())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state              = state_loweringLid;
        printf("Lowering Lid...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_lidCannotMoveStepperBusy, "Can't lower lid.");
    }
}




void openCavity(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't open cavity - Lid not initialized.");
        callbackFunc(callbackRef, err_lidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.door.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't open cavity.  Door is not initialized.");
        callbackFunc(callbackRef, err_lidCannotMoveDoorNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
        return;
    }
    else if(motionData.state != state_idle)
    {
        callbackFunc(callbackRef, err_lidCannotMoveStepperBusy, "Can't open cavity.");
        return;
    }


    if(startRaiseLid())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_openingCavityLid;
        printf("Opening Cavity...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_lidCannotMoveStepperBusy, "Can't open cavity.");
    }
}



void closeCavity(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    ASSERT(callbackFunc);

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.door.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't close cavity.  Door is not initialized.");
        callbackFunc(callbackRef, err_doorNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(!motionData.lid.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't close cavity - Lid not initialized.");
        callbackFunc(callbackRef, err_doorCannotMoveLidNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle && startCloseDoor())
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state = state_closingCavityDoor;
        printf("Closing Cavity...\n");
    }
    else
    {
        callbackFunc(callbackRef, err_doorCannotMoveStepperBusy, "Can't open cavity.");
    }
}



void disableDoor()
{
    motionData.door.initialized = false;
    motionData.door.disabled    = true;
}



void setCurrentDoorClosedPosition()
{
    motionData.door.closedPosition = get_stepper_position(stepperF);
}



void setCurrentLidLoweredPosition()
{
    motionData.lid.compressedPosition = get_stepper_position(stepperU);
}



void setDoorClosedPosition(int position)
{
    if(position < door_min_closed_position ||
       position > door_max_closed_position)
    {
        position = door_default_closed_position;
    }

    motionData.door.closedPosition = position;
}



void setLidLoweredPosition(int position)
{
    if(position < lid_min_compressed_position ||
       position > lid_max_compressed_position)
    {
        position = lid_default_compressed_position;
    }

    motionData.lid.compressedPosition = position;
}



bool getDoorClosedPosition(int *position)
{

    if(*position < door_min_closed_position ||
       *position > door_max_closed_position)
    {
        *position = door_default_closed_position;
        return false;
    }

    return true;
}



bool getLidLoweredPosition(int *position)
{

    if(*position < lid_min_compressed_position ||
       *position > lid_max_compressed_position)
    {
        *position = lid_default_compressed_position;
        return false;
    }

    return true;
}



PlateType getPlateType()
{
    return motionData.lid.plateType;
}



const char* getPlateTypeName()
{
    switch(motionData.lid.plateType)
    {
        case plateType_none:
            return "None";

        case plateType_sda:
            return "SDA";

        case plateType_pcr:
            return "PCR";

        case plateType_unknown:
        default:
            return "Unknown";
    }
}




///////////////////////////////////////////////////
// Local filter functions

static bool verifyDoorPosition(int expectedPosition)
{
    return get_stepper_position(stepperF) == expectedPosition;
}



static bool verifyLidPosition(int expectedPosition)
{
    return get_stepper_position(stepperU) == expectedPosition;
}


static bool verifyDoorOpenedSensor()
{
    return stepper_is_at_home_position(stepperF)                            &&
           get_stepper_position(stepperF) == door_open_position             &&
           (get_stepper_position(stepperF) == motionData.door.startPosition ||
            motionData.door.homePinFound);
}



static bool verifyDoorClosedSensor()
{
    return stepper_is_at_alt_position(stepperF)                             &&
           get_stepper_position(stepperF) == motionData.door.closedPosition &&
           (get_stepper_position(stepperF) == motionData.door.startPosition ||
            motionData.door.altPinFound);
}



static bool verifyLidRaisedSensors()
{
    return stepper_is_at_home_position(stepperU)                           &&
           get_stepper_position(stepperU) == lid_raised_position           &&
           (get_stepper_position(stepperU) == motionData.lid.startPosition ||
            motionData.lid.homePinFound);
}



static bool verifyLidLoweredSensors()
{
    return (get_stepper_position(stepperU) == motionData.lid.compressedPosition    ||
            motionData.lid.plateType == plateType_none)                               &&
           (get_stepper_position(stepperU) == motionData.lid.startPosition         ||
            ((motionData.lid.altPinCount >= 1 && motionData.lid.altPinCount <= 2)     &&
             (motionData.lid.altSpaceCount >= 0 && motionData.lid.altSpaceCount <= 2) &&
             (motionData.lid.altPinCount >= motionData.lid.altSpaceCount)));
}


static void disableAndFinishMotorMove(StepperMotor stepper, ErrorCodes error, const char* errorDesc)
{
    if(error != err_noError)
    {
        stepper_enable(stepper, false);
    }

    finishMotorMove(error, errorDesc);
}

static void finishMotorMove(ErrorCodes error, const char* errorDesc)
{
    motionData.state = state_idle;

    if(motionData.callback.function)
    {
        motionData.callback.function(motionData.callback.reference, error, errorDesc);
    }

    printf("Done.\n");
}



extern char inbyte(void);

void motionTest()
{
    const int       STEP_PER_KEY = 500;
    const StepperMotor STEPPER      = stepperU;

    while(1)
    {
        int c = inbyte();
        if(c == 'u')
        {
            printf("Stepper up...\n");
            stepper_move_relative(STEPPER, -STEP_PER_KEY);
        }
        else if(c == 'd')
        {
            printf("Stepper down...\n");
            stepper_move_relative(STEPPER, STEP_PER_KEY);
        }
        else if(c == 'q')
        {
            break;
        }
    }
}



static bool initProcess()
{
    switch(motionData.init.state)
    {
        case initState_idle:
            return true;

        case initState_filterStart:
            if(timerExpired(&motionData.init.timer))
            {
                if(stepper_busy())
                {
                    finishMotorMove(err_filterInitializationFailure, "Stepper busy - unable to start filter init");
                    return true;
                }

                startTimer(&motionData.init.timer, MSEC_TO_TICKS(filterInitializationTimeout_ms));
                motionData.filter.findSpaceSteps = 0;
                motionData.init.state            = initState_filterFindSpace;
            }
            break;

        case initState_filterFindSpace:
            // Ensure we are not on a filter pin before the search.

            if(timerExpired(&motionData.init.timer))
            {
                finishMotorMove(err_filterInitializationFailure, "Timeout waiting for stepper while finding first space");
                return true;
            }
            else if(!stepper_busy())
            {
                if(!stepper_is_at_home_position(stepperR))
                {
                    if(filterInitRun())
                    {
                        motionData.init.state = initState_filterRun;
                    }
                    else
                    {
                        return true;
                    }
                }
                else if(motionData.filter.findSpaceSteps > maxSpaceFindFilterSteps)
                {
                    finishMotorMove(err_filterInitializationFailure, "Filter sensor error - unable to find first space");
                    return true;
                }
                else
                {
                    motionData.filter.findSpaceSteps += spaceFindStepSize;
                    stepper_move_relative(stepperR, spaceFindStepSize);
                }
            }
            break;

        case initState_filterRun:
            if(!stepper_busy())
            {
                if(filterInitEnd())
                {
                    motionData.init.state = initState_filterEnd;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_filterInitializationFailure, "Filter initialization timed out (step run)");
                return true;
            }
            break;

        case initState_filterEnd:
            if(!stepper_busy())
            {
                motionData.filter.initialized = true;
                printf("Filter Init Done.\n");

                motionData.init.state = initState_completeNotification;
                scheduleTask(notifyInitCompleteTask, NULL, false);

                return false;
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_filterInitializationFailure, "Filter initialization timed out (step end)");
                return true;
            }
            break;

        case initState_lidStart:
            if(!stepper_busy())
            {
                if(lidInitRun1())
                {
                    motionData.init.state = initState_lidRun1;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_lidInitializationFailure, "Lid initialization timed out (step start)");
                return true;
            }
            break;

        case initState_lidRun1:
            if(!stepper_busy())
            {
                if(lidInitRun2())
                {
                    motionData.init.state = initState_lidRun2;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_lidInitializationFailure, "Lid initialization timed out (step run1)");
                return true;
            }
            break;

        case initState_lidRun2:
            if(!stepper_busy())
            {
                if(lidInitEnd())
                {
                    motionData.init.state = initState_lidEnd;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_lidInitializationFailure, "Lid initialization timed out (step run2)");
                return true;
            }
            break;

        case initState_lidEnd:
            if(!stepper_busy())
            {
                motionData.lid.initialized = true;
                printf("Lid Init Done.\n");

                motionData.init.state = initState_completeNotification;
                scheduleTask(notifyInitCompleteTask, NULL, false);

                return false;
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_lidInitializationFailure, "Lid initialization timed out (step end)");
                return true;
            }
            break;

        case initState_doorStart:
            if(!stepper_busy())
            {
                if(doorInitRun())
                {
                    motionData.init.state = initState_doorRun;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_doorInitializationFailure, "Door initialization timed out (step start)");
                return true;
            }
            break;

        case initState_doorRun:
            if(!stepper_busy())
            {
                if(!stepper_is_at_home_position(stepperF))
                {
                    stepper_abort();
                    finishMotorMove(err_doorInitializationFailure, "Home sensor not found");
                    return true;
                }

                if(stepper_is_at_alt_position(stepperF))
                {
                    stepper_abort();
                    finishMotorMove(err_doorInitializationFailure, "Alt sensor not open");
                    return true;
                }

                if(doorInitEnd())
                {
                    motionData.init.state = initState_doorEnd;
                }
                else
                {
                    return true;
                }
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_doorInitializationFailure, "Door initialization timed out (step run)");
                return true;
            }
            break;

        case initState_doorEnd:
            if(!stepper_busy())
            {
                motionData.door.initialized = true;
                printf("Door Init Done.\n");

                motionData.init.state = initState_completeNotification;
                scheduleTask(notifyInitCompleteTask, NULL, false);

                return false;
            }
            else if(timerExpired(&motionData.init.timer))
            {
                stepper_abort();
                finishMotorMove(err_doorInitializationFailure, "Door initialization timed out (step end)");
                return true;
            }
            break;

        case initState_completeNotification:
            // Do nothing
            return true;

        default:
            ASSERT(false);
            break;
    }

    return false;
}



bool notifyInitCompleteTask(void* unused)
{
    if(motionData.init.state == initState_completeNotification)
    {
        motionData.init.state = initState_idle;
        motionData.state      = state_idle;

        if(motionData.callback.function)
        {
            motionData.callback.function(motionData.callback.reference, err_noError, NULL);
        }
    }

    return false;
}



//------------------------------------------------------------------------------
static bool filterInitStart()
{
    // Mark uninitialized & enable stepper
    motionData.filter.initialized = false;
    stepper_enable(stepperR, true);

    printf("Initializing Filter...\n");



    // Slow down the stepper to smooth out the search for the filter pins.
    stepper_set_parameter(stepperR, stepperSlowFrequency, filterPinSearchFrequency);
    stepper_set_parameter(stepperR, stepperFastFrequency, filterPinSearchFrequency);
    stepper_set_parameter(stepperR, stepperRampSteps, filterPinSearchRampSteps);

    stepper_set_home_sensor_make_hook(stepperR, NULL);

    motionData.filter.positionIndex = filterStepsPerRevolution;
    while(motionData.filter.positionIndex)
    {
        motionData.filter.sensorBreakPositions[--motionData.filter.positionIndex] = invalid_position;
    }

    motionData.filter.positionIndex = num_filters;
    while(motionData.filter.positionIndex)
    {
        motionData.filter.filterPositions[--motionData.filter.positionIndex] = invalid_position;
    }


    // Set timer and state
    startTimer(&motionData.init.timer, MSEC_TO_TICKS(initEnableWaitTime_ms));
    motionData.init.state = initState_filterStart;


    return true;
}


static bool filterInitRun()
{
    // Start filter rotation
    stepper_set_home_sensor_make_hook(stepperR, filterInitHomeSensorHook);
    stepper_move_relative(stepperR, filterStepsPerRevolution);

    startTimer(&motionData.init.timer, SEC_TO_TICKS(2 + (filterStepsPerRevolution /
                                                         filterPinSearchFrequency)));

    return true;
}


static bool filterInitEnd()
{
    char errorDesc[ERROR_DESC_SIZE];
    errorDesc[0] = 0;
    
    // Restore parameters for normal stepper operation.
    stepper_set_home_sensor_make_hook(stepperR, NULL);
    stepper_set_parameter(stepperR, stepperSlowFrequency, motionData.filter.slowFrequency);
    stepper_set_parameter(stepperR, stepperFastFrequency, motionData.filter.fastFrequency);
    stepper_set_parameter(stepperR, stepperRampSteps,  motionData.filter.rampSteps);

    if(determineFilterPositions(motionData.filter.sensorBreakPositions,
                                motionData.filter.filterPositions,
                                errorDesc, ERROR_DESC_SIZE))
    {
        stepper_move_to_position(stepperR, motionData.filter.filterPositions[filter_clear]);
        startTimer(&motionData.init.timer, MSEC_TO_TICKS(filterInitializationTimeout_ms));
        return true;
    }

    printf("Filter Init failed.  %s\n", errorDesc);
    finishMotorMove(err_filterInitializationFailure, errorDesc);
    return false;
}



//------------------------------------------------------------------------------
static bool filterInitHomeSensorHook(int position)
{
    motionData.filter.sensorBreakPositions[motionData.filter.positionIndex++] = position;
    motionData.filter.positionIndex %= filterStepsPerRevolution;
    return true;
}

//------------------------------------------------------------------------------
static bool determineFilterPositions(int sensorBreakPositions[], int filterPositions[], char* errorDesc, unsigned errorDescSize)
{
    ASSERT(errorDesc);
    
    int pin_start_positions[numFilterPins];
    int pin_index = 0;
    int *this_break_position = &sensorBreakPositions[0];
    int first_small_distance = invalid_position;
    int home_pin = numFilterPins;
    int filter_index;
    int last_contiguous_position;

    pin_start_positions[0]   = *this_break_position % filterStepsPerRevolution;
    last_contiguous_position = pin_start_positions[0];

    // Find the starting position of each pin by searching for
    // non-contiguous entries in the sensor break array.
    while(*++this_break_position != invalid_position)
    {
        *this_break_position %= filterStepsPerRevolution;

        if(*this_break_position == (last_contiguous_position + 1) % filterStepsPerRevolution)
        {
            last_contiguous_position = *this_break_position;
        }
        else
        {
            if(++pin_index >= numFilterPins)
            {
                snprintf(errorDesc, errorDescSize, "Too many pins found (%d found)", pin_index);
                printf("Filter init error - %s\n", errorDesc);
                return false;
            }

            pin_start_positions[pin_index] =
                last_contiguous_position = *this_break_position;
        }
    }

    if(pin_index + 1 < numFilterPins)
    {
        snprintf(errorDesc, errorDescSize, "Too few pins found (%d found)", pin_index);
        printf("Filter init error - %s\n", errorDesc);
        return false;
    }

    // Find the "home" pin - it lies between the first and last filters.
    for(pin_index = 0; pin_index < numFilterPins - 1; ++pin_index)
    {
        int pin_distance =
            (pin_start_positions[pin_index + 1] -
             pin_start_positions[pin_index]) %
            filterStepsPerRevolution;

        if(pin_distance < 0)
            pin_distance += filterStepsPerRevolution;

        if(pin_distance < minFilterSeparation)
        {
            if(first_small_distance != invalid_position)
            {
                home_pin = pin_index;
                break;
            }
            first_small_distance = pin_index;
        }
    }

    if(home_pin >= numFilterPins)
    {
        int pin_distance =
            (pin_start_positions[0] - pin_start_positions[pin_index]) %
            filterStepsPerRevolution;

        if(pin_distance < 0)
            pin_distance += filterStepsPerRevolution;

        if(pin_distance < minFilterSeparation)
        {
            if(first_small_distance == invalid_position)
            {
                snprintf(errorDesc, errorDescSize, "first_small_distance is invalid");
                printf("Filter init error - %s\n", errorDesc);
                return false;
            }

            if(first_small_distance == 0)
                home_pin = 0;
            else
                home_pin = pin_index;
        }
    }

    // Record the filter positions starting with the filter after the home pin.
    for(filter_index = 0, pin_index = 0; filter_index < num_filters; filter_index++, pin_index++)
    {
        filterPositions[filter_index] =
            pin_start_positions[(home_pin + 1 + pin_index + numFilterPins) % numFilterPins] + 1;
    }

    return true;
}



///////////////////////////////////////////////////
// Local lid functions

static bool lidInitStart()
{
    motionData.lid.initialized = false;
    motionData.init.state = initState_lidStart;

    stepper_set_parameter(stepperU, stepperSlowFrequency, lid_initialization_frequency);
    stepper_set_parameter(stepperU, stepperFastFrequency, lid_initialization_frequency);
    stepper_set_parameter(stepperU, stepperRampSteps, lid_initialization_ramp_steps);
    stepper_set_home_sensor_make_hook(stepperU, NULL);
    stepper_set_alt_sensor_make_hook(stepperU, NULL);
    stepper_set_alt_sensor_break_hook(stepperU, NULL);

    if(!stepper_is_at_alt_position(stepperF))
    {
        printf("Move door first...\n");
        motionData.lid.altPinCount = 0;
        stepper_set_parameter(stepperF, stepperSlowFrequency, door_initialization_frequency);
        stepper_set_parameter(stepperF, stepperFastFrequency, door_initialization_frequency);
        stepper_set_parameter(stepperF, stepperRampSteps, door_initialization_ramp_steps);
        stepper_set_alt_sensor_make_hook(stepperF, lidInitDoorAltSensorMakeCallback);
        stepper_move_relative(stepperF, lid_preinit_door_move_steps);
        startTimer(&motionData.init.timer, MSEC_TO_TICKS(2000));
    }
    else
    {
        startTimer(&motionData.init.timer, 0);
    }

    return true;
}



static bool lidInitRun1()
{
    if(!stepper_is_at_alt_position(stepperU))
    {
        // Move lid down a few steps to make sure we see the home sensor
        stepper_move_relative(stepperU, lid_preinitialization_move_steps);
        startTimer(&motionData.init.timer, MSEC_TO_TICKS(2000));
    }
    else
    {
        startTimer(&motionData.init.timer, 0);
    }

    return true;
}



static bool lidInitRun2()
{
    // Start looking for home flag
    stepper_move_to_position(stepperU, stepperHome);
    startTimer(&motionData.init.timer, MSEC_TO_TICKS(lid_initialization_timeout_ms));
    return true;
}



static bool lidInitEnd()
{
    // Reset speed
    stepper_set_parameter(stepperU, stepperSlowFrequency, motionData.lid.slowFrequency);
    stepper_set_parameter(stepperU, stepperFastFrequency, motionData.lid.fastFrequency);
    stepper_set_parameter(stepperU, stepperRampSteps, motionData.lid.rampSteps);
    stepper_set_home_sensor_make_hook(stepperU, NULL);
    stepper_set_alt_sensor_make_hook(stepperU, NULL);
    stepper_set_alt_sensor_break_hook(stepperU, NULL);

    // Move to home/raised position
    stepper_move_to_position(stepperU, lid_raised_position);
    startTimer(&motionData.init.timer, MSEC_TO_TICKS(lid_initialization_timeout_ms));

    return true;
}



static bool startRaiseLid()
{
    if(!stepper_busy())
    {
        motionData.lid.startPosition    = get_stepper_position(stepperU);
        motionData.lid.plateType        = plateType_unknown;
        motionData.lid.homePinFound     = false;
        motionData.lid.altPinCount      = 0;
        motionData.lid.altSpaceCount    = 0;
        motionData.lid.homePinMakePos   = 0;
        motionData.lid.altPinMake1Pos   = 0;
        motionData.lid.altPinBreak1Pos  = 0;
        motionData.lid.altPinMake2Pos   = 0;
        motionData.lid.altPinBreak2Pos  = 0;

        stepper_set_parameter(stepperU, stepperSlowFrequency, motionData.lid.slowFrequency);
        stepper_set_parameter(stepperU, stepperFastFrequency, motionData.lid.fastFrequency);
        stepper_set_parameter(stepperU, stepperRampSteps, motionData.lid.rampSteps);
        stepper_set_home_sensor_make_hook(stepperU, lidHomeSensorMakeCallback);
        stepper_set_alt_sensor_make_hook(stepperU, NULL);
        stepper_set_alt_sensor_break_hook(stepperU, NULL);

        if(stepper_move_to_position(stepperU, lid_raised_position))
        {
            return true;
        }
    }

    return false;
}


static bool startLowerLid()
{
    if(!stepper_busy())
    {
        motionData.lid.startPosition    = get_stepper_position(stepperU);
        motionData.lid.homePinFound     = false;
        motionData.lid.altPinCount      = 0;
        motionData.lid.altSpaceCount    = 0;
        motionData.lid.homePinMakePos   = 0;
        motionData.lid.altPinMake1Pos   = 0;
        motionData.lid.altPinBreak1Pos  = 0;
        motionData.lid.altPinMake2Pos   = 0;
        motionData.lid.altPinBreak2Pos  = 0;

        stepper_set_parameter(stepperU, stepperSlowFrequency, motionData.lid.slowFrequency);
        stepper_set_parameter(stepperU, stepperFastFrequency, motionData.lid.fastFrequency);
        stepper_set_parameter(stepperU, stepperRampSteps, motionData.lid.rampSteps);

        if(motionData.lid.startPosition > lid_lowered_position)
        {
            // Lid is below the lowered position (in the compressing region)
            // so it will move up to the lowered position.  Don't watch
            // the sensors because we will get invalid sensor readings.
            stepper_set_home_sensor_make_hook(stepperU, NULL);
            stepper_set_alt_sensor_make_hook(stepperU, NULL);
            stepper_set_alt_sensor_break_hook(stepperU, NULL);
        }
        else
        {
            stepper_set_home_sensor_make_hook(stepperU, NULL);
            stepper_set_alt_sensor_make_hook(stepperU, lidAltSensorMakeCallback);
            stepper_set_alt_sensor_break_hook(stepperU, NULL);
        }
        
        if(motionData.lid.startPosition == motionData.lid.compressedPosition)
        {
            motionData.lid.loweringState = lidLoweringState_lowering;
            return true;
        }
        
        if(stepper_move_to_position(stepperU, lid_lowered_position))
        {
            motionData.lid.plateType      = plateType_unknown;
            motionData.lid.loweringState  = lidLoweringState_lowering;
            return true;
        }
    }

    return false;
}



static bool startCompressLid()
{
    if(!stepper_busy())
    {
        stepper_set_parameter(stepperU, stepperSlowFrequency, motionData.lid.slowFrequency);
        stepper_set_parameter(stepperU, stepperFastFrequency, motionData.lid.slowFrequency);
        stepper_set_parameter(stepperU, stepperRampSteps, motionData.lid.rampSteps);
        stepper_set_home_sensor_make_hook(stepperU, NULL);

        if(stepper_is_at_alt_position(stepperU))
        {
            if(motionData.lid.altPinCount == 0)
            {
                //Pin sensed, but because of starting position, it was missed.
                motionData.lid.altPinCount = 1;
            }
            stepper_set_alt_sensor_make_hook(stepperU, NULL);
            stepper_set_alt_sensor_break_hook(stepperU, lidAltSensorBreakCallback);
        }
        else
        {
            stepper_set_alt_sensor_make_hook(stepperU, lidAltSensorMakeCallback);
            stepper_set_alt_sensor_break_hook(stepperU, NULL);
        }

        if(stepper_move_to_position(stepperU, motionData.lid.compressedPosition))
        {
            motionData.lid.loweringState  = lidLoweringState_compressing;
            return true;
        }
    }

    return false;
}



static bool lidHomeSensorMakeCallback(int position)
{
    stepper_set_home_sensor_make_hook(stepperU, NULL);

    motionData.lid.homePinMakePos = position;

    if(position - stepper_get_home_pin_location(stepperU) > lid_max_home_sensor_steps)
    {
        motionData.state = state_error;
        motionData.error = err_lidRaiseFailure;

        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                 "Raised pin found at pos %d, should be %d +/- %d",
                 position, stepper_get_home_pin_location(stepperU), lid_max_home_sensor_steps);
        puts(motionData.errorDesc);
        
        return false;
    }

    printf("Lid home pin at %d\n", position);
    motionData.lid.homePinFound = true;
    stepper_reset_home_pin_location();

    return true;
}



static bool lidAltSensorMakeCallback(int position)
{
    stepper_set_alt_sensor_make_hook(stepperU, NULL);

    if(motionData.lid.altPinCount == 0)
    {
        motionData.lid.altPinMake1Pos = position;
    }
    else if(motionData.lid.altPinCount == 1)
    {
        motionData.lid.altPinMake2Pos = position;
    }

    if(motionData.lid.compressedPosition - position > lid_max_alt_sensor_steps)
    {
        motionData.state = state_error;
        motionData.error = err_lidLowerFailure;

        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                 "Lower pin found at pos %d, should be %d +/- %d",
                 position, motionData.lid.compressedPosition, lid_max_alt_sensor_steps);
        puts(motionData.errorDesc);
        
        return false;
    }

    printf("Lid alt pin %d at %d", motionData.lid.altPinCount, position);
    ++motionData.lid.altPinCount;

    if(motionData.lid.altPinCount < 2)
    {
        motionData.lid.plateType = plateType_sda;
        printf(" (SDA)\n");
    }
    else if(motionData.lid.altPinCount == 2)
    {
        motionData.lid.plateType = plateType_pcr;
        printf(" (PCR)\n");
    }
    else
    {
        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                 "Too many lower pins (%d) at pos %d", motionData.lid.altPinCount, position);
        printf("\n%s\n", motionData.errorDesc);

        return false;
    }

    stepper_set_alt_sensor_break_hook(stepperU, lidAltSensorBreakCallback);

    return true;
}



static bool lidAltSensorBreakCallback(int position)
{
    stepper_set_alt_sensor_break_hook(stepperU, NULL);

    if(motionData.lid.altSpaceCount == 0)
    {
        motionData.lid.altPinBreak1Pos = position;
    }
    else if(motionData.lid.altSpaceCount == 1)
    {
        motionData.lid.altPinBreak2Pos = position;
    }

    printf("Lid alt space %d at %d\n", motionData.lid.altSpaceCount, position);
    ++motionData.lid.altSpaceCount;
    
#if BETA_HARDWARE_SUPPORT
//TODO: Temporarily ignore space between SDA & PCR - Call it SDA
//      and space below PCR - call it PCR.
//      Need mechanical adjustment to fix issue
        stepper_set_alt_sensor_make_hook(stepperU, lidAltSensorMakeCallback);
#else
    if(motionData.lid.altSpaceCount < 2)
    {
        //Okay to continue
        motionData.lid.plateType = plateType_unknown;
        stepper_set_alt_sensor_make_hook(stepperU, lidAltSensorMakeCallback);
    }
    else if(motionData.lid.altSpaceCount == 2)
    {
        //Second space found (No plate present) - stop so we don't run into the electroform
        printf("No plate present\n");
        motionData.lid.plateType = plateType_none;
        return false;
    }
    else
    {
        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                 "Too many lower spaces (%d) at pos %d", motionData.lid.altSpaceCount, position);
        puts(motionData.errorDesc);

        return false;
    }
#endif

    return true;
}



static bool lidInitDoorAltSensorMakeCallback(int position)
{
    if(++motionData.lid.altPinCount >= lid_preinit_door_steps_past_alt)
    {
        //Door moved to alt sensor - okay now to init lid
        stepper_set_home_sensor_make_hook(stepperF, NULL);
        return false;
    }
    return true;
}



///////////////////////////////////////////////////
// Local door functions

static bool doorInitStart()
{
    motionData.door.initialized = false;

    if(motionData.door.disabled)
    {
        finishMotorMove(err_doorInitializationFailure, "Can't init door, currently disabled");
        return false;
    }

    if(!motionData.lid.initialized)
    {
        finishMotorMove(err_doorInitializationFailure, "Can't init door, lid not initialized");
        return false;
    }

    motionData.init.state = initState_doorStart;


    // Set speed for initialization
    stepper_set_parameter(stepperF, stepperSlowFrequency, door_initialization_frequency);
    stepper_set_parameter(stepperF, stepperFastFrequency, door_initialization_frequency);
    stepper_set_parameter(stepperF, stepperRampSteps, door_initialization_ramp_steps);
    stepper_set_home_sensor_make_hook(stepperF, NULL);
    stepper_set_alt_sensor_make_hook(stepperF, NULL);



    // Move door to correctly find home flag
    if(!stepper_is_at_alt_position(stepperF))
    {
        // Move door out a few steps to make sure we see the home sensor
        stepper_move_relative(stepperF, door_preinitialization_move_steps);
        startTimer(&motionData.init.timer, MSEC_TO_TICKS(2000));
    }
    else
    {
        if(stepper_is_at_home_position(stepperF))
        {
            finishMotorMove(err_doorInitializationFailure, "Home and Alt sensors blocked");
            return false;
        }
        startTimer(&motionData.init.timer, 0);
    }

    return true;
}



static bool doorInitRun()
{
    // Start looking for home flag
    stepper_move_to_position(stepperF, stepperHome);
    startTimer(&motionData.init.timer, MSEC_TO_TICKS(door_initialization_timeout_ms));
    return true;
}



static bool doorInitEnd()
{
    // Reset door speed
    stepper_set_parameter(stepperF, stepperSlowFrequency, motionData.door.slowFrequency);
    stepper_set_parameter(stepperF, stepperFastFrequency, motionData.door.fastFrequency);
    stepper_set_parameter(stepperF, stepperRampSteps, motionData.door.rampSteps);
    stepper_set_home_sensor_make_hook(stepperF, NULL);
    stepper_set_alt_sensor_make_hook(stepperF, NULL);

    // Move to home/open position
    stepper_move_to_position(stepperF, door_open_position);
    startTimer(&motionData.init.timer, MSEC_TO_TICKS(door_initialization_timeout_ms));

    return true;
}


static bool startOpenDoor()
{
    if(!stepper_busy())
    {
        motionData.door.startPosition    = get_stepper_position(stepperF);
        motionData.door.homePinFound     = false;
        motionData.door.altPinFound      = false;
        motionData.door.homePinFoundPos  = 0;
        motionData.door.altPinFoundPos   = 0;

        stepper_set_parameter(stepperF, stepperSlowFrequency, motionData.door.slowFrequency);
        stepper_set_parameter(stepperF, stepperFastFrequency, motionData.door.fastFrequency);
        stepper_set_parameter(stepperF, stepperRampSteps, motionData.door.rampSteps);
        stepper_set_home_sensor_make_hook(stepperF, doorOpenHomeSensorCallback);
        stepper_set_alt_sensor_make_hook(stepperF, NULL);

        if(stepper_move_to_position(stepperF, door_open_position))
        {
            return true;
        }
    }

    return false;
}


static bool startCloseDoor()
{
    if(!stepper_busy())
    {
        motionData.door.startPosition    = get_stepper_position(stepperF);
        motionData.door.homePinFound     = false;
        motionData.door.altPinFound      = false;
        motionData.door.homePinFoundPos  = 0;
        motionData.door.altPinFoundPos   = 0;

        stepper_set_parameter(stepperF, stepperSlowFrequency, motionData.door.slowFrequency);
        stepper_set_parameter(stepperF, stepperFastFrequency, motionData.door.fastFrequency);
        stepper_set_parameter(stepperF, stepperRampSteps, motionData.door.rampSteps);
        stepper_set_home_sensor_make_hook(stepperF, NULL);
        stepper_set_alt_sensor_make_hook(stepperF, doorCloseAltSensorMakeCallback);

        if(stepper_move_to_position(stepperF, motionData.door.closedPosition))
        {
            return true;
        }
    }

    return false;
}



static bool doorOpenHomeSensorCallback(int position)
{
    stepper_set_home_sensor_make_hook(stepperF, NULL);

    motionData.door.homePinFoundPos = position;

    if(position - stepper_get_home_pin_location(stepperF) > door_max_home_sensor_steps)
    {
        motionData.state = state_error;
        motionData.error = err_doorOpenFailure;

        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                 "Open pin found at pos %d, should be %d +/- %d",
                  position, stepper_get_home_pin_location(stepperF), door_max_home_sensor_steps);
        puts(motionData.errorDesc);
        
        return false;
    }

    printf("Door home pin at %d\n", position);
    motionData.door.homePinFound = true;
    stepper_reset_home_pin_location();

    return true;
}



static bool doorCloseAltSensorMakeCallback(int position)
{
    stepper_set_alt_sensor_make_hook(stepperF, NULL);

    motionData.door.altPinFoundPos = position;

    if(motionData.door.closedPosition - position > door_max_alt_sensor_steps)
    {
        motionData.state = state_error;
        motionData.error = err_doorCloseFailure;

        snprintf(motionData.errorDesc, sizeof(motionData.errorDesc),
                "Close pin found at pos %d, should be %d +/- %d",
                position, motionData.door.closedPosition, door_max_alt_sensor_steps);
        puts(motionData.errorDesc);
        
        return false;
    }

    printf("Door alt pin at %d\n", position);
    motionData.door.altPinFound = true;

    return true;
}

void RotateArm(StepperMotor stepperMotor, RotationDirection direction, TurnSize turnSize, int rotations)
{
    armMotionData.motor           = stepperMotor;
    armMotionData.direction       = direction;
    armMotionData.turnSize        = turnSize;
    armMotionData.currentRotation = 0;
    armMotionData.totalRotations  = rotations;

    startTimer(&armMotionData.timer, MSEC_TO_TICKS(armMotionTimeout_ms));
    startTimer(&armMotionData.actuateTimer, MSEC_TO_TICKS(actuateTimeout_ms));

    switch(armMotionData.motor)
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

        default:
            break;
    }

    motionData.state = state_actuatingArmIn;
}


///////////////////////////////////////////////////
// Debug functions

int getDoorStartPosition(void)
{
    return motionData.door.startPosition;
}


bool getDoorHomePinFound(void)
{
    return motionData.door.homePinFound;
}


bool getDoorAltPinFound(void)
{
    return motionData.door.altPinFound;
}


int getDoorHomePinFoundPos(void)
{
    return motionData.door.homePinFoundPos;
}


int getDoorAltPinFoundPos(void)
{
    return motionData.door.altPinFoundPos;
}


int getLidStartPosition(void)
{
    return motionData.lid.startPosition;
}


bool getLidHomePinFound(void)
{
    return motionData.lid.homePinFound;
}


int getLidAltPinCount(void)
{
    return motionData.lid.altPinCount;
}


int getLidAltSpaceCount(void)
{
    return motionData.lid.altSpaceCount;
}


int getLidHomePinMakePos(void)
{
    return motionData.lid.homePinMakePos;
}


int getLidAltPinMake1Pos(void)
{
    return motionData.lid.altPinMake1Pos;
}


int getLidAltPinBreak1Pos(void)
{
    return motionData.lid.altPinBreak1Pos;
}


int getLidAltPinMake2Pos(void)
{
    return motionData.lid.altPinMake2Pos;
}


int getLidAltPinBreak2Pos(void)
{
    return motionData.lid.altPinBreak2Pos;
}


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


// EOF
