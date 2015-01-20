/////////////////////////////////////////////////////////////
//
//  reboot.c
//
//  Reboot utility
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <reboot.h>
#include <processor.h>



void reboot()
{
    disableInterrupts();
    __asm("brai 0");
}



// EOF
