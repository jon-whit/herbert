/////////////////////////////////////////////////////////////
//
//  processor.c
//
//  Xilinx Microblaze Processor Utilities
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert




#include <processor.h>
#include <xparameters.h>
#include <timer.h>
#include <lcd.h>



///////////////////////////////////////////////////
// Local types and macros

#define __mfs()                                  \
    ({                                           \
        unsigned status;                         \
        __asm__ __volatile__ ("mfs	%0, rmsr"    \
             : "=r" (status));                   \
        status;                                  \
    })





///////////////////////////////////////////////////
// Interface functions

CRData enterCriticalRegion()
{
    CRData crdata = !!(__mfs() & 0x0002);
    CLI();
    return crdata;
}



void exitCriticalRegion(CRData crdata)
{
    if(crdata)
    {
        SEI();
    }
}



// EOF
