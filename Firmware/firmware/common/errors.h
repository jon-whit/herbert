//-----------------------------------------------------------------------------
//! \file
//!
//! \brief HR Amp Error Codes
//!
//! This file uses the "error_list.h" file to generate an enumerated type
//! containing the error codes.
//!
//-----------------------------------------------------------------------------

#ifndef _ERRORS_H_
#define _ERRORS_H_

#ifdef DEF_ERROR
#undef DEF_ERROR
#endif

#define DEF_ERROR( name, string ) name,

typedef enum
{
    #include "error_list.h"
    num_errors
} ErrorCodes;

extern char const * const error_strings[];

#endif //...#ifndef _ERRORS_H_
