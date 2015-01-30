///////////////////////////////////////////////////////
//
//  types.h
//
//  Data types

#ifndef types_h
#define types_h

#include <vector>
#include <Exceptions.h>


#ifndef __cplusplus
#ifndef bool
typedef enum { false = 0, true = 1 } bool;
#endif
#endif


typedef unsigned char  uint8;
typedef   signed char  int8;
typedef unsigned short uint16;
typedef   signed short int16;
typedef unsigned long  uint32;
typedef   signed long  int32;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SET 1
#define CLR 0

#define HIGH 1
#define LOW  0

#define __MIN(a,b)  ((a) < (b) ? (a) : (b))
#define __MAX(a,b)  ((a) > (b) ? (a) : (b))

#define STRINGIFY(s)    TOSTRING(s)
#define TOSTRING(s)     #s


typedef std::vector<char> char_vector;

#endif

