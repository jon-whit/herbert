/////////////////////////////////////////////////////////////
//
//  timer.h
//
//  System Timers and Timer ISR
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

#ifndef timer_h
#define timer_h

#include <types.h>
#include <xparameters.h>



///////////////////////////////
// Timer Data Types

typedef struct
{
    volatile uint64 tickCount;
    volatile uint64 initialTimerVal;
} Timer;


typedef void (*PeriodicTimerCallBack) (void* callBackRef);


typedef struct
{
    PeriodicTimerCallBack func;
    void *                ref;
} TimerCallBackInfo;


///////////////////////////////
// Macros

#define USEC_TO_TICKS(usec)  ((XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000000) * ((uint64)(usec)))
#define MSEC_TO_TICKS(msec)  ((XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000) * ((uint64)(msec)))
#define SEC_TO_TICKS(sec)    (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ * ((uint64)(sec)))

#define TICKS_TO_USEC(ticks) (((uint64)(ticks)) / (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000000))
#define TICKS_TO_MSEC(ticks) (((uint64)(ticks)) / (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000))
#define TICKS_TO_SEC(ticks)  (((uint64)(ticks)) / XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ)



///////////////////////////////
// Interface Functions

void timerInit();

uint64 getSystemTicks();

void startTimer(Timer* timer, uint64 ticks);
bool timerExpired(Timer* timer);

void udelay(uint32 delay_us);
void mdelay(uint32 delay_ms);


void enablePeriodicTimer(PeriodicTimerCallBack callBackFunc, void* callBackRef,
                         uint32 ticks, TimerCallBackInfo * currentCallBackInfo);
void disablePeriodicTimer();

uint64 getTimer_ms(Timer* timer);

#endif
