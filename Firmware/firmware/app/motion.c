/////////////////////////////////////////////////////////////
//
//  motion.c
//

#include <motion.h>

#include <string.h>
#include <stdio.h>

#include <timer.h>
#include <comm.h>
#include <os.h>

#include <steppers.h>
#include <stepper_hw.h>
#include <relay.h>
#include <switch.h>

///////////////////////////////////////////////////
// Constants

#define ERROR_DESC_SIZE 200

enum ArmMotionConstants
{
    halfTurnSteps                    = 100, 
    quarterTurnStepsClockwise        = 50,
    quarterTurnStepsCounterClockwise = -50,
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
    state_correctArmAngle,
    state_armDoneSpinning,
    state_actuatingArmOut,
    state_finishMove,
} MotionControlState;

typedef enum
{
    initState_idle,
    initState_ActuateArmsOut,
    initState_motorR,
    initState_motorF,
    initState_motorU,
    initState_motorL,
    initState_motorB,
    initState_motorD,
    initState_completeNotification,
} InitStates;

typedef enum 
{
    rotation_clockwise,
    rotation_counterClockwise,
    rotation_180,
} Rotation;

typedef struct 
{
    StepperMotor motor;
    bool         initialized;
    int          slowFrequency;
    int          fastFrequency;
    int          rampSteps;
} MotorData;

typedef struct
{
    SwitchLocation inSwitch;
    SwitchLocation outSwitch;
    MotorData      motorData;
} ArmData;

typedef struct 
{
    Move     move;
    ArmData  armData;
    Rotation rotation;
    int      correctionSteps;
    bool     isCorrectionDirectionForward;
    int      fullTime;
    int      armFinishedActuatingInTime;
    int      armFinishSpinningTime;
} MoveData;

typedef struct
{
    Timer      timer;
    InitStates state;
} InitData;

typedef struct
{
    OfflineTaskCompleteCallback function;
    int                         reference;
} OperationCallback;

///////////////////////////////////////////////////
// Local function prototypes

static bool initProcess();

static void spinArm(void);
static void finishMotorMove(ErrorCodes error, const char* errorDesc);

static bool areArmDataEqual(ArmData armData1, ArmData armData2);
static void setArmDataEqual(ArmData* armData1, ArmData* armData2);

///////////////////////////////////////////////////
// Local data

static struct MotionData
{
    InitData           init;

    ArmData            arm_R;
    ArmData            arm_F;
    ArmData            arm_U;
    ArmData            arm_L;
    ArmData            arm_B;
    ArmData            arm_D;

    // int                moveCount;

    MoveData           moveData;
    Timer              timer;
    MotionControlState state;
    OperationCallback  callback;
    ErrorCodes         error;
    char               errorDesc[ERROR_DESC_SIZE];
} motionData;

///////////////////////////////////////////////////
// Interface functions

void motionInit()
{
    memset(&motionData, 0, sizeof(motionData));

    motionData.init.state = initState_ActuateArmsOut;
    motionData.state      = state_initializing;

    printf("\nMotion Init...\n");

    motionRestoreDefaults();

    motionData.arm_R.motorData.initialized = false;
    motionData.arm_F.motorData.initialized = false;
    motionData.arm_U.motorData.initialized = false;
    motionData.arm_L.motorData.initialized = false;
    motionData.arm_B.motorData.initialized = false;
    motionData.arm_D.motorData.initialized = false;

    stepper_init();
}

void motionRestoreDefaults(void)
{
    motionData.arm_R.motorData.motor         = stepperR;
    motionData.arm_R.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_R.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_R.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_R.inSwitch                = RInSwitch;
    motionData.arm_R.outSwitch               = BOutSwitch; // Fix

    motionData.arm_F.motorData.motor         = stepperF;
    motionData.arm_F.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_F.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_F.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_F.inSwitch                = FInSwitch;
    motionData.arm_F.outSwitch               = FOutSwitch;

    motionData.arm_U.motorData.motor         = stepperU;
    motionData.arm_U.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_U.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_U.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_U.inSwitch                = UInSwitch;
    motionData.arm_U.outSwitch               = UOutSwitch;

    motionData.arm_L.motorData.motor         = stepperL;
    motionData.arm_L.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_L.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_L.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_L.inSwitch                = LInSwitch;
    motionData.arm_L.outSwitch               = LOutSwitch;

    motionData.arm_B.motorData.motor         = stepperB;
    motionData.arm_B.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_B.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_B.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_B.inSwitch                = BInSwitch;
    motionData.arm_B.outSwitch               = BOutSwitch;

    motionData.arm_D.motorData.motor         = stepperD;
    motionData.arm_D.motorData.slowFrequency = defaultSlowFrequency;
    motionData.arm_D.motorData.fastFrequency = defaultFastFrequency;
    motionData.arm_D.motorData.rampSteps     = defaultRampSteps;
    motionData.arm_D.inSwitch                = DInSwitch;
    motionData.arm_D.outSwitch               = DOutSwitch;
}

void motionAbort()
{
    stepper_abort();
    motionData.state = state_aborting;
}

void initSystem(OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    motionData.state              = state_initializing;
    motionData.callback.function  = callbackFunc;
    motionData.callback.reference = callbackRef;
}

void executeMove(Move move, OfflineTaskCompleteCallback callbackFunc, int callbackRef)
{
    motionData.moveData.move                         = move;
    motionData.moveData.correctionSteps              = 0;
    motionData.moveData.isCorrectionDirectionForward = true;
    // motionData.moveCount                             = 0;

    switch(move)
    {
    case move_RClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_R); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_RCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_R); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_R2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_R); motionData.moveData.rotation = rotation_180;              break;

    case move_FClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_F); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_FCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_F); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_F2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_F); motionData.moveData.rotation = rotation_180;              break;
    
    case move_UClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_U); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_UCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_U); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_U2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_U); motionData.moveData.rotation = rotation_180;              break;

    case move_LClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_L); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_LCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_L); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_L2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_L); motionData.moveData.rotation = rotation_180;              break;

    case move_BClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_B); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_BCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_B); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_B2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_B); motionData.moveData.rotation = rotation_180;              break;

    case move_DClockwise:        setArmDataEqual(&motionData.moveData.armData, &motionData.arm_D); motionData.moveData.rotation = rotation_clockwise;        break;
    case move_DCounterClockwise: setArmDataEqual(&motionData.moveData.armData, &motionData.arm_D); motionData.moveData.rotation = rotation_counterClockwise; break;
    case move_D2:                setArmDataEqual(&motionData.moveData.armData, &motionData.arm_D); motionData.moveData.rotation = rotation_180;              break;
    }

    char errorDesc[ERROR_DESC_SIZE];

    if(!motionData.moveData.armData.motorData.initialized)
    {
        snprintf(errorDesc, ERROR_DESC_SIZE, "Can't Execute Move. Arm Not Initialized.");
        callbackFunc(callbackRef, err_doorNotInitialized, errorDesc);
        printf("%s\n", errorDesc);
        return;
    }

    if(motionData.state == state_aborting)
    {
        callbackFunc(callbackRef, err_systemBusy, "Motion abort in progress.");
    }
    else if(motionData.state == state_idle)
    {
        motionData.callback.function  = callbackFunc;
        motionData.callback.reference = callbackRef;
        motionData.state              = state_actuatingArmIn;

        startTimer(&motionData.timer,          MSEC_TO_TICKS(0));

        if     (areArmDataEqual(motionData.moveData.armData, motionData.arm_R)) { actuateArmIn('R'); }
        else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_F)) { actuateArmIn('F'); }
        else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_U)) { actuateArmIn('U'); }
        else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_L)) { actuateArmIn('L'); }
        else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_B)) { actuateArmIn('B'); }
        else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_D)) { actuateArmIn('D'); }
    }
    else
    {
        callbackFunc(callbackRef, err_doorCannotMoveStepperBusy, "Can't perform move.");
    }
}


bool motionProcess(void* unused)
{
    switch(motionData.state)
    {
    case state_idle:
        break;

    case state_initializing:
        if(initProcess())
        {
            motionData.state = state_idle;
        }
        break;

    case state_error:
        finishMotorMove(motionData.error, motionData.errorDesc);
        break;

    case state_aborting:
        motionData.state = state_idle;
        break;

    case state_actuatingArmIn:
        if(IsSwitchTriggered(motionData.moveData.armData.inSwitch) && !IsSwitchTriggered(motionData.moveData.armData.outSwitch))
        {
            motionData.moveData.correctionSteps = 0;
            motionData.moveData.armFinishedActuatingInTime = getTimer_ms(&motionData.timer);
            motionData.state = state_armActuatedIn;
        }
        break;

    case state_armActuatedIn:
        spinArm();
        break;

    case state_armSpinning:
        if(!stepper_busy())
        {
            if(!isSensorBeamBroken(stepperR))
            {
                motionData.moveData.armFinishSpinningTime = getTimer_ms(&motionData.timer);
                motionData.state = state_armDoneSpinning;
            }
            else
            {
                motionData.state = state_correctArmAngle;
            }
        }
        break;

    case state_correctArmAngle:
        if(motionData.moveData.isCorrectionDirectionForward)
        {
            stepper_move_relative(motionData.moveData.armData.motorData.motor, motionData.moveData.correctionSteps);
            motionData.moveData.isCorrectionDirectionForward = false;
            motionData.moveData.correctionSteps += 1;
        }
        else
        {
            stepper_move_relative(motionData.moveData.armData.motorData.motor, (0 - motionData.moveData.correctionSteps));
            motionData.moveData.isCorrectionDirectionForward = true;
            motionData.moveData.correctionSteps += 1;
        }
        motionData.state = state_armSpinning;
        break;

    case state_armDoneSpinning:
        stepper_abort();
        actuateAllArmsOut();
        motionData.state = state_actuatingArmOut;
        break;            

    case state_actuatingArmOut:
        if(IsSwitchTriggered(motionData.moveData.armData.outSwitch) && !IsSwitchTriggered(motionData.moveData.armData.inSwitch))
        {
            // motionData.moveCount += 1;
            // if(motionData.moveCount <= 1)
            // {
            //     if     (areArmDataEqual(motionData.moveData.armData, motionData.arm_R)) { actuateArmIn('R'); }
            //     else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_F)) { actuateArmIn('F'); }
            //     else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_U)) { actuateArmIn('U'); }
            //     else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_L)) { actuateArmIn('L'); }
            //     else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_B)) { actuateArmIn('B'); }
            //     else if(areArmDataEqual(motionData.moveData.armData, motionData.arm_D)) { actuateArmIn('D'); }
            //     motionData.state = state_actuatingArmIn;
            // }
            // else
            // {
                motionData.moveData.fullTime = getTimer_ms(&motionData.timer);
                printf("Arm Timing\n"
                       "\tFull Time              - %d\n"
                       "\tArm Actuating In Time  - %d\n"
                       "\tArm Spinning Time      - %d\n"
                       "\tArm Actuating Out Time - %d\n", 
                       motionData.moveData.fullTime, 
                       motionData.moveData.armFinishedActuatingInTime,
                       motionData.moveData.armFinishSpinningTime - motionData.moveData.armFinishedActuatingInTime,
                       motionData.moveData.fullTime - motionData.moveData.armFinishSpinningTime);

                disableMotors();

                motionData.state = state_finishMove;
            // }
        }
        break;

    case state_finishMove:
        motionData.callback.function(motionData.callback.reference, err_noError, NULL);
        motionData.state = state_idle;
        break;

    default:
        motionData.state = state_idle;
    }

    return true;
}

/////////// Static Functions //////////

static void finishMotorMove(ErrorCodes error, const char* errorDesc)
{
    motionData.state = state_idle;

    if(motionData.callback.function)
    {
        motionData.callback.function(motionData.callback.reference, error, errorDesc);
    }
}

static bool initProcess()
{
    switch(motionData.init.state)
    {
        case initState_idle:
            return true;

        case initState_ActuateArmsOut:
            actuateAllArmsOut();
            if(IsSwitchTriggered(motionData.arm_B.outSwitch) /*&& IsSwitchTriggered(motionData.arm_F.outSwitch) && IsSwitchTriggered(motionData.arm_U.outSwitch) && 
               IsSwitchTriggered(motionData.arm_L.outSwitch) && IsSwitchTriggered(motionData.arm_B.outSwitch) && IsSwitchTriggered(motionData.arm_D.outSwitch)*/)
            {
                motionData.init.state = initState_motorR;
            }
            break;

        case initState_motorR: 
            if(!isSensorBeamBroken(stepperR)) { /*motionData.init.state = initState_motorF;*/ motionData.init.state = initState_completeNotification; } else { stepper_move_relative(motionData.arm_R.motorData.motor, 1); } break;
        // case initState_motorF: 
        //     if(!isSensorBeamBroken(stepperF)) { motionData.init.state = initState_motorU; } else { stepper_move_relative(motionData.arm_F.motorData.motor, 1); } break;
        // case initState_motorU: 
        //     if(!isSensorBeamBroken(stepperU)) { motionData.init.state = initState_motorL; } else { stepper_move_relative(motionData.arm_U.motorData.motor, 1); } break;
        // case initState_motorL: 
        //     if(!isSensorBeamBroken(stepperL)) { motionData.init.state = initState_motorB; } else { stepper_move_relative(motionData.arm_L.motorData.motor, 1); } break;
        // case initState_motorB: 
        //     if(!isSensorBeamBroken(stepperB)) { motionData.init.state = initState_motorD; } else { stepper_move_relative(motionData.arm_B.motorData.motor, 1); } break;
        // case initState_motorD: 
        //     if(!isSensorBeamBroken(stepperD)) { motionData.init.state = initState_completeNotification; } else { stepper_move_relative(motionData.arm_D.motorData.motor, 1); } break;

        case initState_completeNotification:
            motionData.init.state = initState_idle;
            motionData.state      = state_idle;

            printf("Arms Initialized\n");

            motionData.arm_R.motorData.initialized = true;
            // motionData.arm_F.motorData.initialized = true;
            // motionData.arm_U.motorData.initialized = true;
            // motionData.arm_L.motorData.initialized = true;
            // motionData.arm_B.motorData.initialized = true;
            // motionData.arm_D.motorData.initialized = true;

            if(motionData.callback.function)
            {
                motionData.callback.function(motionData.callback.reference, err_noError, NULL);
            }
            return true;

            default:
            ASSERT(false);
            break;
    }

    return false;
}

static void spinArm(void)
{
    if(motionData.moveData.rotation == rotation_clockwise)
    {
        stepper_move_relative(motionData.moveData.armData.motorData.motor, quarterTurnStepsClockwise);
    }
    else if(motionData.moveData.rotation == rotation_counterClockwise)
    {
        stepper_move_relative(motionData.moveData.armData.motorData.motor, quarterTurnStepsCounterClockwise);
    }
    else if(motionData.moveData.rotation == rotation_180)
    {
        stepper_move_relative(motionData.moveData.armData.motorData.motor, halfTurnSteps);
    }

    motionData.state = state_armSpinning;
}

static bool areArmDataEqual(ArmData armData1, ArmData armData2)
{
    return armData1.inSwitch                == armData2.inSwitch                &&
           armData1.outSwitch               == armData2.outSwitch               &&
           armData1.motorData.motor         == armData2.motorData.motor         &&
           armData1.motorData.initialized   == armData2.motorData.initialized   &&
           armData1.motorData.slowFrequency == armData2.motorData.slowFrequency &&
           armData1.motorData.fastFrequency == armData2.motorData.fastFrequency &&
           armData1.motorData.rampSteps     == armData2.motorData.rampSteps;
}

static void setArmDataEqual(ArmData* armData1, ArmData* armData2)
{
    armData1->inSwitch                = armData2->inSwitch;
    armData1->outSwitch               = armData2->outSwitch;
    armData1->motorData.motor         = armData2->motorData.motor;
    armData1->motorData.initialized   = armData2->motorData.initialized;
    armData1->motorData.slowFrequency = armData2->motorData.slowFrequency;
    armData1->motorData.fastFrequency = armData2->motorData.fastFrequency;
    armData1->motorData.rampSteps     = armData2->motorData.rampSteps;
}

// EOF
