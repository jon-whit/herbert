/////////////////////////////////////////////////////////////
//
//  assert.h
//
//  Assert implemetation
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#ifndef assert_h
#define assert_h


#include <processor.h>


void  printAssert(char* expr, char* fileName, int lineNum);



#define ASSERT(expr) { if(!(expr)){ CLI(); printAssert(#expr, __FILE__, __LINE__); while(1){} } }



#endif
