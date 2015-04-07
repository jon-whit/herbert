//-----------------------------------------------------------------------------
//! \file
//!
//! \brief HR Amp Error Strings
//!
//! This file uses the "error_list.h" file to generate a table of strings
//! containing the error descriptions.
//!
//-----------------------------------------------------------------------------

#include "errors.h"

#ifdef DEF_ERROR
#undef DEF_ERROR
#endif

#define DEF_ERROR( name, string ) string,

char const * const error_strings[] =
{
    #include "error_list.h"
    ""
};
