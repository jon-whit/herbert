/////////////////////////////////////////////////////////////
//
//  StringFormatting.h
//
//  std string formatting functions
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef StringFormatting_h
#define StringFormatting_h

#include <string>
#include <vector>
#include <Exceptions.h>


const char defaultWhiteSpaceDelimiters[] = " \t\r\n";

std::string formatString(const char* format, ...);

std::string formatBinaryData(const std::string& data, int leftPad = 0, int columnCount = 16, int colSeparatorPos = 8);
std::string formatBinaryData(const std::vector<char>& data, int leftPad = 0, int columnCount = 16, int colSeparatorPos = 8);
std::string formatBinaryData(const char* data, int dataLen, int leftPad = 0, int columnCount = 16, int colSeparatorPos = 8);

std::string trimString(const std::string& src, const std::string& c = defaultWhiteSpaceDelimiters);
std::string unquoteString(const std::string& s);
std::string quoteString(const std::string& s);
bool        isStringQuoted(const std::string& s);
std::string concatString(const std::string& s1, const std::string& s2);

bool naturalStringCompare(const std::string& left, const std::string& right);



class ParamException : public std::runtime_error
{
public:
    inline ParamException(const std::string& ex) : std::runtime_error(ex) {}
};

unsigned    stringParamCount(const std::string& params, const std::string& delimiters = defaultWhiteSpaceDelimiters);
std::string nextParam(std::string& params, const std::string& delimiters = defaultWhiteSpaceDelimiters);
std::string stringParam(std::string& params, const std::string& delimiters = defaultWhiteSpaceDelimiters);
int         intParam(std::string& params, int radix = 0, const std::string& delimiters = defaultWhiteSpaceDelimiters);
unsigned    unsignedParam(std::string& params, int radix = 0, const std::string& delimiters = defaultWhiteSpaceDelimiters);
double      doubleParam(std::string& params, const std::string& delimiters = defaultWhiteSpaceDelimiters);

int         stringToInt(const std::string& string, int radix = 0, const std::string& delimiters = defaultWhiteSpaceDelimiters);
unsigned    stringToUnsigned(const std::string& string, int radix = 0, const std::string& delimiters = defaultWhiteSpaceDelimiters);
double      stringToDouble(const std::string& string, const std::string& delimiters = defaultWhiteSpaceDelimiters);

#endif
