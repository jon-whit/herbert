/////////////////////////////////////////////////////////////
//
//  timer.c
//
//  System Timers and Timer ISR
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

#include <timer.h>
#include <xparameters.h>
#include <interrupt.h>
#include <processor.h>



///////////////////////////////////////////////////
// Constants

#define TSCR_ENALL_MASK 0x0400
#define TSCR_PWM_MASK   0x0200
#define TSCR_TINT_MASK  0x0100
#define TSCR_ENT_MASK   0x0080
#define TSCR_ENIT_MASK  0x0040
#define TSCR_LOAD_MASK  0x0020
#define TSCR_ARHT_MASK  0x0010
#define TSCR_CAPT_MASK  0x0008
#define TSCR_GENT_MASK  0x0004
#define TSCR_UDT_MASK   0x0002
#define TSCR_MDT_MASK   0x0001



///////////////////////////////////////////////////
// Local types and macros

#define TCSR0_REG     (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x00)))
#define TLR0_REG      (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x04)))
#define TCR0_REG      (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x08)))
#define TCSR1_REG     (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x10)))
#define TLR1_REG      (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x14)))
#define TCR1_REG      (*((volatile unsigned *)(XPAR_TIMER_BASEADDR + 0x18)))



///////////////////////////////////////////////////
// Local function prototypes

static void timerIsr(void *callbackRef);



///////////////////////////////////////////////////
// Local data

PeriodicTimerCallBack periodicTimerCallBackFunc = NULL;
void*                 periodicTimercallBackRef  = NULL;
uint32                highTimer                 = 0;


///////////////////////////////////////////////////
// Interface functions

void timerInit()
{
    TLR0_REG  = 0;
    TCR0_REG  = 0;
    highTimer = 0;
    
    TCSR0_REG = TSCR_ENALL_MASK |
                TSCR_ENT_MASK   |
                TSCR_TINT_MASK  |
                TSCR_ENIT_MASK  |
                TSCR_ARHT_MASK;

    TCSR1_REG = 0;
                    
    registerInterruptHandler(XPAR_INTC_TIMER_INTERRUPT_INTR, timerIsr, 0);
    enableInterrupt(XPAR_INTC_TIMER_INTERRUPT_INTR);
}



uint64 getSystemTicks()
{
    uint32 high;
    uint32 low;
    uint32 highCheck;

    do
    {
        high = highTimer;
        low  = TCR0_REG;

        CRData crdata = enterCriticalRegion();
        {
            timerIsr( NULL );
        }
        exitCriticalRegion(crdata);
    
        highCheck = highTimer;
    } while( high != highCheck );

    return ((uint64)high << 32) | low;
}


void startTimer(Timer* timer, uint64 ticks)
{
    timer->tickCount       = ticks;
    timer->initialTimerVal = getSystemTicks();
}


bool timerExpired(Timer* timer)
{
    return (getSystemTicks() - timer->initialTimerVal) >= timer->tickCount;
}

uint64 getTimer_ms(Timer* timer)
{
    return TICKS_TO_MSEC(getSystemTicks() - timer->initialTimerVal);
}

void udelay(uint32 delay_us)
{
    Timer timer;
    
    startTimer(&timer, USEC_TO_TICKS(delay_us));
    
    while(!timerExpired(&timer))
    {
        //Wait for timer to expire
    }
}



void mdelay(uint32 delay_ms)
{
    Timer timer;
    
    startTimer(&timer, MSEC_TO_TICKS(delay_ms));
    
    while(!timerExpired(&timer))
    {
        //Wait for timer to expire
    }
}



void enablePeriodicTimer(PeriodicTimerCallBack callBackFunc, void* callBackRef,
                         uint32 ticks, TimerCallBackInfo * currentCallBackInfo)
{
    if( currentCallBackInfo )
    {
        currentCallBackInfo->func = periodicTimerCallBackFunc;
        currentCallBackInfo->ref  = periodicTimercallBackRef;
    }

    CRData crdata = enterCriticalRegion();
    {
        TLR1_REG = ticks;
        TCR1_REG = ticks;
    
        TCSR1_REG = TSCR_ENALL_MASK |
                    TSCR_ENT_MASK   |
                    TSCR_TINT_MASK  |
                    TSCR_ENIT_MASK  |
                    TSCR_ARHT_MASK  |
                    TSCR_UDT_MASK;
        
        periodicTimerCallBackFunc = callBackFunc;
        periodicTimercallBackRef  = callBackRef;
    }
    exitCriticalRegion(crdata);
}



void disablePeriodicTimer()
{
    CRData crdata = enterCriticalRegion();
    {
        TCSR1_REG = 0;

        periodicTimerCallBackFunc = NULL;
        periodicTimercallBackRef  = NULL;
    }
    exitCriticalRegion(crdata);
}



///////////////////////////////////////////////////
// Local functions

static void timerIsr(void *callbackRef)
{
    if( TCSR0_REG & TSCR_TINT_MASK )
    {
        // Clear interrupt
        TCSR0_REG |= TSCR_TINT_MASK;

        highTimer++;
    }

    if( TCSR1_REG & TSCR_TINT_MASK )
    {
        // Clear interrupt
        TCSR1_REG |= TSCR_TINT_MASK;

        if(periodicTimerCallBackFunc)
        {
            periodicTimerCallBackFunc(periodicTimercallBackRef);
        }
    }
}






// EOF
