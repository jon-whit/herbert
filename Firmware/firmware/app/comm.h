/////////////////////////////////////////////////////////////
//
//  comm.h
//
//  Communication Protocol Processor

#ifndef comm_h
#define comm_h

#include <types.h>
#include <errors.h>


typedef void (*OfflineTaskCompleteCallback) (int reference, ErrorCodes error, const char* errorDesc);
typedef void (*OfflineTaskInfoCallback)     (int reference, ErrorCodes error, const char* errorDesc);



void commInit();
bool commProcess(void* unused);

void sendErrorMsg(ErrorCodes errorCode, const char* errorDesc);
void sendLogMsg(const char* logMsg);



#endif
