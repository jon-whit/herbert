/////////////////////////////////////////////////////////////
//
//  signal.c
//
//  Signal mechanism - used to signal events, etc. from ISR
//  to app code.
//
//  Similar in functionality to a semaphore in a threaded system,
//  but the calls are not blocking and must be polled.

#include <signal.h>
#include <processor.h>
#include <assert.h>



///////////////////////////////////////////////////
// Interface functions

void initSignal(Signal* signal, uint32 initialCount, uint32 maxCount)
{
    ASSERT(signal);
    ASSERT(maxCount > 0);
    ASSERT(initialCount <= maxCount);

    CRData crdata = enterCriticalRegion();
    {
        signal->count    = initialCount;
        signal->maxCount = maxCount;
    }
    exitCriticalRegion(crdata);
}



void setSignal(Signal* signal)
{
    ASSERT(signal);

    CRData crdata = enterCriticalRegion();
    {
        if(signal->count < signal->maxCount)
        {
            signal->count++;
        }
    }
    exitCriticalRegion(crdata);
}



bool getSignal(Signal* signal)
{
    ASSERT(signal);

    bool signaled = false;

    CRData crdata = enterCriticalRegion();
    {
        if(signal->count)
        {
            signal->count--;
            signaled = true;
        }
    }
    exitCriticalRegion(crdata);

    return signaled;
}



// EOF
