/////////////////////////////////////////////////////////////
//
//  interrupt.h
//
//  Interrupt module interface
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert


#ifndef interrupt_h
#define interrupt_h


#include <types.h>



////////////////////////////
// Types

typedef void (*InterruptHandler) (void* callBackRef);



////////////////////////////
// API Functions

void intcInit();

void enableInterrupt(uint8 interruptId);
void disableInterrupt(uint8 interruptId);

bool registerInterruptHandler(uint8 interruptId, InterruptHandler handler, void* callBackRef);


#endif
