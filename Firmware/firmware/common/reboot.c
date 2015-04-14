/////////////////////////////////////////////////////////////
//
//  reboot.c
//
//  Reboot utility
//

#include <reboot.h>
#include <processor.h>



void reboot()
{
    disableInterrupts();
    __asm("brai 0");
}



// EOF
