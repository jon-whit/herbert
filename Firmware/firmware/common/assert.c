/////////////////////////////////////////////////////////////
//
//  assert.c
//
//  Assert implemetation

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <system.h>
#include <timer.h>
#include <xuartlite_l.h>



///////////////////////////////////////////////////
// Interface functions

uint8 waitForKey()
{
    Timer ledTimer;
    
    startTimer(&ledTimer, 0);

	while(XUartLite_mIsReceiveEmpty(STDIN_BASEADDRESS))
	{
        if(timerExpired(&ledTimer))
        {
            SYSTEM.heartbeatLED = ~SYSTEM.heartbeatLED;
            startTimer(&ledTimer, MSEC_TO_TICKS(HEARTBEAT_ERROR_PERIOD_ms));
        }
	}

	return (uint8)XUartLite_mReadReg(STDIN_BASEADDRESS, XUL_RX_FIFO_OFFSET);
}



void printAssert(char* expr, char* fileName, int lineNum)
{
    CRData crdata = enterCriticalRegion();
    {
        do
        {
            printf("\n\n\nAssertion Failure:\n");
            printf("  '%s'\n", expr);
            printf("  %s @ Line %d\n", fileName, lineNum);
        } while(waitForKey() != 0x03);  // Continue on Ctrl-C
    
        printf("\nContinuing after Assert\n");
    }
    exitCriticalRegion(crdata);
}





// EOF
