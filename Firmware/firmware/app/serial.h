/////////////////////////////////////////////////////////////
//
//  serial.h
//
//  Serial driver for the command protocol

#ifndef serial_h
#define serial_h

#include <types.h>
#include <signal.h>


#define MAX_RX_DATA_SIZE  1024
#define MAX_TX_DATA_SIZE  1024


typedef enum
{
    SBC_SERIAL_PORT,
    DEBUG_SERIAL_PORT
} SerialPort;


void     serialInit();
void     setSerialPortRxSignal(SerialPort port, Signal* signal);

unsigned serialGetPkt(SerialPort port, uint8* pkt, unsigned maxLength);
bool     serialSendPkt(SerialPort port, const uint8* pkt, unsigned length, uint32 timeout_us);


#endif
