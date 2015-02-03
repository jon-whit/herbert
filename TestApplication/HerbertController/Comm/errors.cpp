//-----------------------------------------------------------------------------
//! \file
//!
//! \brief HR Amp Error Strings
//!
//! This file uses the "error_list.h" file to generate a table of strings
//! containing the error descriptions.
//!
//! Copyright (c) 2006 Idaho Technology Inc.
//-----------------------------------------------------------------------------

#include "errors.h"

#ifdef DEF_ERROR
#undef DEF_ERROR
#endif

#include <Exceptions.h>


#define DEF_ERROR( name, string ) string,

char const * const error_strings[] =
{
    #include "error_list.h"
    ""
};

const char* GetErrorString(unsigned error)
{
    ITI_ASSERT_LESS(static_cast<ErrorCodes>(error), num_errors);

    return error_strings[error];
}

