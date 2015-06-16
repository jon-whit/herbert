/////////////////////////////////////////////////////////////
//
//  motion.h
//
//  High-level instrument motion control for door, lid and filters
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef motion_h
#define motion_h

#include <comm.h>
#include <steppers.h>

typedef enum
{
    move_RClockwise,
    move_RCounterClockwise,
    move_R2,
    move_FClockwise,
    move_FCounterClockwise,
    move_F2,
    move_UClockwise,
    move_UCounterClockwise,
    move_U2,
    move_LClockwise,
    move_LCounterClockwise,
    move_L2,
    move_BClockwise,
    move_BCounterClockwise,
    move_B2,
    move_DClockwise,
    move_DCounterClockwise,
    move_D2,
} Move;

void motionInit(void);
void motionAbort(void);
bool motionProcess(void* unused);
void motionRestoreDefaults(void);

void initSystem(OfflineTaskCompleteCallback callbackFunc, int callbackRef);

void executeMove(Move move, OfflineTaskCompleteCallback callbackFunc, int callbackRef);

#endif
