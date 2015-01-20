/////////////////////////////////////////////////////////////
//
//  os.h
//
//  Simple cooperative task scheduler/processor
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert
//
//
//  Task functions take a void* as a parameter and
//  return a bool - true = run again, false - don't run again.
//
//  Each task should be cooperative by exiting within a reasonable
//  amount of time to allow other tasks to run.
//
//  scheduleTask() is ISR safe
//

#ifndef os_h
#define os_h

#include <types.h>


typedef bool (*Task) (void* reference);


void osInit();
void osStart();
bool scheduleTask(Task task, void* reference, bool high);


#endif
