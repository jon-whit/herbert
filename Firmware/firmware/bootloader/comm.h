/////////////////////////////////////////////////////////////
//
//  comm.h
//
//  Communication Protocol Processor
//

#ifndef comm_h
#define comm_h


void commInit();

void commProcess();

void sendLogMsg(const char *format, ...);


#endif
