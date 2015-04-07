/////////////////////////////////////////////////////////////
//
//  main.c
//
//  Main processing loop

#include <startup.h>
#include <assert.h>


int main (void)
{
    // Start up system - never returns
    systemStartup();

    // Catch errors
    ASSERT(false);    

    return 0;
}

