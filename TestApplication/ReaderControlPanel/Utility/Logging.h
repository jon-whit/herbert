/////////////////////////////////////////////////////////////
//
//  Logging.h
//
//  Logging server
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#ifndef Logging_h
#define Logging_h


#include <string>


int log(const std::string& str);
int log(const char* format, ...);




#endif
