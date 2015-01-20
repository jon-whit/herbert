/////////////////////////////////////////////////////////////
//
//  types.h
//
//  Common type defines
//
//  Copyright 2008 Idaho Technology
//  Created by Brett Gilbert

#ifndef types_h
#define types_h


typedef unsigned char  uint8;
typedef   signed char   int8;

typedef unsigned short uint16;
typedef   signed short  int16;

typedef unsigned long  uint32;
typedef   signed long   int32;

typedef unsigned long long int uint64;
typedef   signed long long int int64;

#ifndef bool
    typedef enum { false = 0, true = 1 } bool;
#endif

#ifndef NULL
    #define NULL 0
#endif


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


#define SET 1
#define CLR 0


#define STRINGIFY(s) STR(s)
#define STR(s) #s


#endif
