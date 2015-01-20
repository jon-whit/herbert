/////////////////////////////////////////////////////////////
//
//  vectors.c
//
//  Exception Vectors utility functions
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <vectors.h>
#include <types.h>



///////////////////////////////////////////////////
// Constants

extern uint32 __vectors;
extern uint32 _exception_handler;
extern uint32 _hw_exception_handler;

void _interrupt_handler(void);


uint32* VECTORS = (uint32*)&__vectors;

const uint32 exceptionHandler   = (uint32)&_exception_handler;
const uint32 interruptHandler   = (uint32)&_interrupt_handler;
const uint32 hwExceptionHandler = (uint32)&_hw_exception_handler;


#define ASM_BRAI 0xB8080000
#define ASM_IMM  0xB0000000


#define SET_VECTOR(vec_addr, handler_addr)   { *((vec_addr))   = (ASM_IMM  | ((handler_addr) >> 16)); \
                                               *((vec_addr)+1) = (ASM_BRAI | ((handler_addr) & 0x0000ffff)); }



#define EXCEPTION_VECTOR_INDEX    2
#define INTERRUPT_VECTOR_INDEX    4
#define HW_EXCEPTION_VECTOR_INDEX 8



///////////////////////////////////////////////////
// Interface functions

void loadExceptionVectors()
{
    SET_VECTOR(&VECTORS[EXCEPTION_VECTOR_INDEX], exceptionHandler);
    SET_VECTOR(&VECTORS[INTERRUPT_VECTOR_INDEX], interruptHandler);
    SET_VECTOR(&VECTORS[HW_EXCEPTION_VECTOR_INDEX], hwExceptionHandler);
}


// EOF
