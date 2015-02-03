#ifndef __CRC32_H__
#define __CRC32_H__



#include <stddef.h>



#ifdef __cplusplus
    extern "C" {
#endif



#define INITIAL_CRC32_VALUE 0xFFFFFFFFUL



typedef unsigned long CRC32;



CRC32 calcCRC32(CRC32 initialCrc, const void* bufPtr, size_t length);



#ifdef __cplusplus
    } // extern "C"
#endif



#endif
