/////////////////////////////////////////////////////////////
//
//  main.c
//
//  Main processing loop
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

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

