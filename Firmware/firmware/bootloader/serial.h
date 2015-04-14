/////////////////////////////////////////////////////////////
//
//  serial.h
//
//  Serial driver for the command protocol

#ifndef serial_h
#define serial_h

#include <types.h>


void     serialInit();
void     serialProcess();
bool     txProcessing();

unsigned serialGetPkt(uint8* pkt, unsigned maxLength);
bool     serialSendPkt(uint8* pkt, unsigned length);


#endif
