/////////////////////////////////////////////////////////////
//
//  interrupt.c
//
//  Interrupt module interface
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

#include <interrupt.h>
#include <xintc.h>

#include <lcd.h>
#include <processor.h>
#include <assert.h>



///////////////////////////////////////////////////
// Constants



///////////////////////////////////////////////////
// Local types and macros



///////////////////////////////////////////////////
// Local function prototypes



///////////////////////////////////////////////////
// Local data

static XIntc interruptController;



///////////////////////////////////////////////////
// Interface functions

void intcInit()
{
    XStatus status;

    status = XIntc_Initialize(&interruptController, XPAR_INTC_0_DEVICE_ID);
    ASSERT(status == XST_SUCCESS);

    status = XIntc_Start(&interruptController, XIN_REAL_MODE /*XIN_SIMULATION_MODE*/);
    ASSERT(status == XST_SUCCESS);
}



bool registerInterruptHandler(uint8 interruptId, InterruptHandler handler, void* callBackRef)
{
    return XIntc_Connect(&interruptController, interruptId, (XInterruptHandler)handler, callBackRef) == XST_SUCCESS;
}



void enableInterrupt(uint8 interruptId)
{
    XIntc_Enable(&interruptController, interruptId);
}



void disableInterrupt(uint8 interruptId)
{
    XIntc_Disable(&interruptController, interruptId);
}



///////////////////////////////////////////////////
// Local functions



// EOF
