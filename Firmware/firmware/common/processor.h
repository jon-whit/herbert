/////////////////////////////////////////////////////////////
//
//  processor.h
//
//  Xilinx Microblaze Processor Utilities
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#ifndef processor_h
#define processor_h


#include <types.h>



////////////////////////////
// Macros

#define SEI()                                    \
    {                                            \
      __asm__ __volatile__ ("msrset  r0, 0x2;"   \
             :                                   \
             :                                   \
             : "memory");                        \
    }

#define CLI()                                    \
    {                                            \
      __asm__ __volatile__ ("msrclr r0, 0x2;"    \
             :                                   \
             :                                   \
             : "memory");                        \
    }

#define NOP() __asm("NOP")



#define enableInterrupts()  SEI()
#define disableInterrupts() CLI()



////////////////////////////
// Types

typedef bool CRData;



////////////////////////////
// API Functions

CRData enterCriticalRegion();
void   exitCriticalRegion(CRData crdata);





#endif
