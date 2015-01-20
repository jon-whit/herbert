/////////////////////////////////////////////////////////////
//
//  comm.h
//
//  Communication Protocol Processor
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

#ifndef comm_h
#define comm_h


void commInit();

void commProcess();

void sendLogMsg(const char *format, ...);


#endif
