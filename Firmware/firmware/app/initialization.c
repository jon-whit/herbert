/////////////////////////////////////////////////////////////
//
//  initialization.c
//
//  System initialization
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <initialization.h>


#include <stdio.h>
#include <types.h>
#include <interrupt.h>
#include <lcd.h>
#include <timer.h>
#include <processor.h>
#include <assert.h>
#include <serial.h>
#include <comm.h>
#include <version.h>
#include <cache.h>
#include <AT45DB161D.h>
#include <motion.h>
#include <os.h>
#include <system.h>
#include <relay.h>



///////////////////////////////////////////////////
// Constants



///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    OfflineTaskCompleteCallback completedCallbackFunc;
    OfflineTaskInfoCallback     infoCallbackFunc;
    int                         reference;
} OperationCallback;



typedef enum
{
    MotionState_Idle,
    MotionState_FilterInit,
    MotionState_LidInit,
    MotionState_DoorInit,
    MotionState_Complete,
} MotionInitStates;


typedef struct
{
    MotionInitStates  state;
    ErrorCodes        error;
} MotionInit;



typedef struct
{
    MotionInit        motion;
    OperationCallback callback;
    bool              haltOnError;
} InitData;



///////////////////////////////////////////////////
// Local function prototypes

static bool startInitTask(void* unused);
static void sendTaskInfo(int reference, ErrorCodes error, const char* errorDesc);
static void finishInitTask(ErrorCodes error, const char* errorDesc);

// static void motionInitStart();
// static void motionInitProcess();
// static void motionInitCallback(int reference, ErrorCodes error, const char* errorDesc);



///////////////////////////////////////////////////
// Local data

static InitData initData;


///////////////////////////////////////////////////
// Interface functions

void initializationInit()
{
    initData.callback.completedCallbackFunc = NULL;
    initData.callback.infoCallbackFunc      = NULL;
}



void resetSystemToDefaults()
{
    // set defaults
}



void startSystemInitialization(OfflineTaskCompleteCallback completedCallbackFunc,
                               OfflineTaskInfoCallback     infoCallbackFunc,
                               int                         callbackRef,
                               bool                        haltOnError)
{
    ASSERT(completedCallbackFunc);

    if(initData.callback.completedCallbackFunc)
    {
        completedCallbackFunc(callbackRef, err_systemBusy, "Init system busy");
        return;
    }
    
    initData.callback.completedCallbackFunc = completedCallbackFunc;
    initData.callback.infoCallbackFunc      = infoCallbackFunc;
    initData.callback.reference             = callbackRef;
    initData.haltOnError                    = haltOnError;

    scheduleTask(startInitTask, NULL, false);
}


void initializationAbort()
{
    if(initData.callback.completedCallbackFunc)
    {
        initData.callback.completedCallbackFunc = NULL;
        initData.callback.infoCallbackFunc      = NULL;
        initData.motion.state                   = MotionState_Idle;

        printf("System Init Aborted\n");
    }
}





///////////////////////////////////////////////////
// Local functions

static bool startInitTask(void* unused)
{
    // Kick off all parallel initializations
    //motionInitStart();

    return false;
}


static void sendTaskInfo(int reference, ErrorCodes error, const char* errorDesc)
{
    if(initData.callback.infoCallbackFunc)
    {
        initData.callback.infoCallbackFunc(reference, error, errorDesc);
    }
}


static void finishInitTask(ErrorCodes error, const char* errorDesc)
{
    if(initData.callback.completedCallbackFunc)
    {
        initData.callback.completedCallbackFunc(initData.callback.reference, error, errorDesc);
        initData.callback.completedCallbackFunc = NULL;
    }
}



///////////////////////////////////////////////////
// Motion Init Functions - Sequential Init Steps

// static void motionInitStart()
// {
//     motionInitProcess();
// }

// static void motionInitProcess()
// {
//     initData.motion.state = MotionState_Complete;
//     switch(initData.motion.state)
//     {
//         case MotionState_Idle:
//             initData.motion.state = MotionState_Complete;
//             break;

//         case MotionState_FilterInit:
//             printf("Filter init...\n");
        
//             break;

//         case MotionState_LidInit:
//             printf("Lid init...\n");

//             break;

//         case MotionState_DoorInit:
//             printf("Door init...\n");

//             break;

//         case MotionState_Complete:
//             initData.motion.state = MotionState_Idle;
//             finishInitTask(initData.motion.error, NULL);
//             break;
            
//         default:
//             ASSERT(false);
//     }
// }

// static void motionInitCallback(int reference, ErrorCodes error, const char* errorDesc)
// {
//      switch(initData.motion.state)
//     {
//         case MotionState_Idle:
//             break;

//         case MotionState_FilterInit:


//             break;


//         case MotionState_LidInit:


//             break;

//         case MotionState_DoorInit:

//             break;

//         case MotionState_Complete:
//             break;
            
//         default:
//             ASSERT(false);
//     }

//     motionInitProcess();
// }


// EOF
