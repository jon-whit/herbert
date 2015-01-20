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



///////////////////////////////////////////////////
// Local data



///////////////////////////////////////////////////
// Interface functions

void timerInit()
{
    TLR0_REG = 0;
    TCR0_REG = 0;
    
    TCSR0_REG = TSCR_ENALL_MASK |
                TSCR_ENT_MASK   |
                TSCR_ARHT_MASK;

    TCSR1_REG = 0;
}



void startTimer(Timer* timer, uint32 ticks)
{
    timer->tickCount       = ticks;
    timer->initialTimerVal = TCR0_REG;
}



bool timerExpired(Timer* timer)
{
    return (TCR0_REG - timer->initialTimerVal) >= timer->tickCount;
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





///////////////////////////////////////////////////
// Local functions



// EOF
