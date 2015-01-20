#ifndef __CRC16_H__
#define __CRC16_H__


#include <stdlib.h>


#ifdef __cplusplus
    extern "C" {
#endif



#define INITIAL_CRC16_VALUE 0xFFFF


typedef unsigned short CRC16;


CRC16 calcCRC16(CRC16 initialCrc, const char *pStr, size_t lenStr);



#ifdef __cplusplus
    } // extern "C"
#endif

#endif
