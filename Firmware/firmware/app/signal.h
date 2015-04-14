/////////////////////////////////////////////////////////////
//
//  signal.h
//
//  Signal mechanism - used to signal events, etc. from ISR
//  to app code.
//
//  Similar in functionality to a semaphore in a threaded system,
//  but the calls are not blocking and must be polled.

#ifndef signal_h
#define signal_h

#include <types.h>


typedef struct
{
    volatile uint32 count;
    uint32          maxCount;
} Signal;



void initSignal(Signal* signal, uint32 initialCount, uint32 maxCount);
void setSignal(Signal* signal);
bool getSignal(Signal* signal);



#endif
