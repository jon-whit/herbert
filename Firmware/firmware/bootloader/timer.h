/////////////////////////////////////////////////////////////
//
//  timer.h
//
//  System Timers and Timer ISR
//

#ifndef timer_h
#define timer_h

#include <types.h>
#include <xparameters.h>



///////////////////////////////
// Timer Data Types

typedef struct
{
    volatile uint32 tickCount;
    volatile uint32 initialTimerVal;
} Timer;



typedef void (*PeriodicTimerCallBack) (void* callBackRef);



///////////////////////////////
// Macros

#define USEC_TO_TICKS(usec)  ((XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000000) * (usec))
#define MSEC_TO_TICKS(msec)  ((XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000) * (msec))
#define SEC_TO_TICKS(sec)    (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ * ((uint32)sec))



///////////////////////////////
// Interface Functions

void timerInit();
void udelay(uint32 delay_us);
void mdelay(uint32 delay_ms);


void startTimer(Timer* timer, uint32 ticks);
bool timerExpired(Timer* timer);


#endif
