/////////////////////////////////////////////////////////////
//
//  StringFormatting.cpp
//
//  std string formatting functions
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <StringFormatting.h>

#include <string>
#include <sstream>
#include <stdarg.h>
#include <math.h>


#if 0

static void stringUnitTest()
{
    ASSERT(naturalStringCompare("abc", "abc"));
    ASSERT(naturalStringCompare("1", "1"));
    ASSERT(naturalStringCompare("1", "2"));
    ASSERT(!naturalStringCompare("2", "1"));
    ASSERT(naturalStringCompare("2", "10"));
    ASSERT(!naturalStringCompare("10", "2"));
    ASSERT(naturalStringCompare("abc1", "abc1"));
    ASSERT(naturalStringCompare("abc2", "abc10"));
    ASSERT(!naturalStringCompare("abc10", "abc2"));
    ASSERT(naturalStringCompare("abc1abc", "abc1abc"));
    ASSERT(naturalStringCompare("abc2abc", "abc10abc"));
    ASSERT(!naturalStringCompare("abc10abc", "abc2abc"));
    ASSERT(naturalStringCompare("abc100abc", "abc100abc1"));
    ASSERT(naturalStringCompare("abc100abc0", "abc100abc1"));
    ASSERT(!naturalStringCompare("abc100abc1", "abc100abc0"));
    ASSERT(naturalStringCompare("abc100abc010", "abc100abc011"));
    ASSERT(!naturalStringCompare("abc100abc11", "abc100abc000001"));
    ASSERT(naturalStringCompare("abc10def", "abc010def"));
    ASSERT(!naturalStringCompare("abc010def", "abc10def"));
    ASSERT(naturalStringCompare("abc010def2", "abc10def10"));
    ASSERT(!naturalStringCompare("abc10def10", "abc010def2"));
    ASSERT(!naturalStringCompare("abc010def10", "abc10def010"));
    ASSERT(naturalStringCompare("abc10def010", "abc010def10"));
}

class RunStringUnitTest
{
public:
    RunStringUnitTest() { stringUnitTest(); }
} runTest;

#endif





std::string formatString(const char* format, ...)
{
    const int STR_ITERATION_SIZE = 10000;
    
    std::string str;
    int strLen;

    do
    {
        str.resize(str.size() + STR_ITERATION_SIZE);

        va_list argList;

        va_start(argList, format);
        strLen = _vsnprintf(&str.at(0), str.size() - 1, format, argList);
        va_end(argList);
    } while(strLen < 0);

    str.resize(strLen);

    return str;
}




std::string formatBinaryData(const std::string& data, int leftPad, int columnCount, int colSeparatorPos)
{
    if(data.size() == 0)
    {
        return "";
    }

    return formatBinaryData(data.c_str(), data.size(), leftPad, columnCount, colSeparatorPos);
}


std::string formatBinaryData(const std::vector<char>& data, int leftPad, int columnCount, int colSeparatorPos)
{
    if(data.size() == 0)
    {
        return "";
    }

    return formatBinaryData(&data.at(0), data.size(), leftPad, columnCount, colSeparatorPos);
}


std::string formatBinaryData(const char* data, int dataLen, int leftPad, int columnCount, int colSeparatorPos)
{
    std::stringstream s;

    for(int i = 0; i < dataLen; i += columnCount)
    {
        for(int pad = 0; pad < leftPad; pad++)
        {
            s << " ";
        }

        int byte;
        for(byte = 0; byte < columnCount; byte ++)
        {
            if(byte > 0)
            {
                if(colSeparatorPos && (byte % colSeparatorPos) == 0)
                {
                    s << "  ";
                }
                else
                {
                    s << " ";
                }
            }



            if(i + byte < dataLen)
            {
                s << formatString("%02x", data[i + byte] & 0x000000ff);
            }
            else
            {
                s << "  ";
            }
        }

        s << "  ";

        for(byte = 0; byte < columnCount && i + byte < dataLen; byte ++)
        {
            if(byte > 0 && colSeparatorPos && (byte % colSeparatorPos) == 0)
            {
                s << " ";
            }

            if(i + byte < dataLen)
            {
                unsigned char val = static_cast<unsigned char>(data[i + byte]) & 0x000000ff;

                s << (char)(isprint(val) ? val : '.');
            }
        }

        s << std::endl;
    }

    return s.str();
}


std::string trimString(const std::string& src, const std::string& c)
{
    size_t p2 = src.find_last_not_of(c);
    if (p2 == std::string::npos) return std::string();
    size_t p1 = src.find_first_not_of(c);
    if (p1 == std::string::npos) p1 = 0;
    return src.substr(p1, (p2-p1)+1);
}


std::string unquoteString(const std::string& s)
{
    size_t pos1 = s.find('"');
    size_t pos2 = s.rfind('"');

    if(s.size() >= 2 && pos1 == 0 && pos2 == s.size() - 1)
    {
        return s.substr(1, s.size() - 2);
    }

    return s;
}


std::string quoteString(const std::string& s)
{
    if(s.find_first_of(defaultWhiteSpaceDelimiters) != std::string::npos)
    {
        return '"' + s + '"';
    }

    return s;
}


bool isStringQuoted(const std::string& s)
{
    size_t pos1 = s.find('"');
    size_t pos2 = s.rfind('"');

    return s.size() >= 2 && pos1 == 0 && pos2 == s.size() - 1;
}



std::string concatString(const std::string& s1, const std::string& s2)
{
    return quoteString(unquoteString(s1) + unquoteString(s2));
}


bool naturalStringCompare(const std::string& left, const std::string& right)
{
    std::string::const_iterator leftPos  = left.begin();
    std::string::const_iterator rightPos = right.begin();

    unsigned leftNumScore  = 0;
    unsigned rightNumScore = 0;

    while(leftPos != left.end() && rightPos != right.end())
    {
        if(isdigit(*leftPos) && isdigit(*rightPos))
        {
            char* leftNumEnd;
            char* rightNumEnd;

            unsigned leftNum  = strtoul(&(*leftPos),  &leftNumEnd, 10);
            unsigned rightNum = strtoul(&(*rightPos), &rightNumEnd, 10);

            if(leftNum < rightNum)
            {
                return true;
            }
            else if(leftNum > rightNum)
            {
                return false;
            }

            unsigned leftNumSize  = leftNumEnd  - &(*leftPos);
            unsigned rightNumSize = rightNumEnd - &(*rightPos);

            leftPos  += leftNumSize;
            rightPos += rightNumSize;

            if(leftNumSize < rightNumSize)
            {
                leftNumScore  <<= 1;
                rightNumScore <<= 1;

                rightNumScore += 1;
            }
            else if(leftNumSize > rightNumSize)
            {
                leftNumScore  <<= 1;
                rightNumScore <<= 1;

                leftNumScore += 1;
            }
        }
        else
        {
            if(*leftPos < *rightPos)
            {
                return true;
            }
            else if(*leftPos > *rightPos)
            {
                return false;
            }

            ++leftPos;
            ++rightPos;
        }
    }

    if(leftPos == left.end())
    {
        if(rightPos == right.end() && leftNumScore > rightNumScore)
        {
            return false;
        }
        return true;
    }

    return false;
}


unsigned stringParamCount(const std::string& params, const std::string& delimiters)
{
    std::string paramsCopy = params;
    unsigned count = 0;

    while(nextParam(paramsCopy, delimiters).size())
    {
        ++count;
    }

    return count;
}


std::string nextParam(std::string& params, const std::string& delimiters)
{
    std::string param;
    size_t      pos;

    params = trimString(params);

    if(params.empty()) return std::string();

    if(params[0] == '"' && delimiters.find('"') == std::string::npos)
    {
        pos = params.find('"', 1);

        if(pos == std::string::npos)
        {
            param = params;
            params.clear();
        }
        else
        {
            param = params.substr(1, pos - 1);
            params.erase(0, pos + 1);
        }
    }
    else
    {
        pos = params.find_first_of(delimiters);

        if(pos != std::string::npos)
        {
            param  = params.substr(0, pos);
            params.erase(0, pos + 1);
        }
        else
        {
            param = params;
            params.clear();
        }
    }

    params = trimString(params);

    return param;
}



std::string stringParam(std::string& params, const std::string& delimiters)
{
    std::string param = nextParam(params, delimiters);

    if(param.empty())
    {
        throw ParamException("Missing parameter");
    }

    return param;
}



int intParam(std::string& params, int radix, const std::string& delimiters)
{
    std::string param = nextParam(params, delimiters);

    if(param.empty())
    {
        throw ParamException("Missing parameter");
    }

    char* endPtr;

    int val = strtol(param.c_str(), &endPtr, radix);

    if(*endPtr)
    {
        throw ParamException("Invalid integer value - '" + param + "'");
    }

    if((val == LONG_MAX || val == LONG_MIN) && errno == ERANGE)
    {
        throw ParamException("Integer value out of range - '" + param + "'");
    }

    return val;
}



unsigned unsignedParam(std::string& params, int radix, const std::string& delimiters)
{
    std::string param = nextParam(params, delimiters);

    if(param.empty())
    {
        throw ParamException("Missing parameter");
    }

    char* endPtr;

    unsigned val = strtoul(param.c_str(), &endPtr, radix);

    if(*endPtr)
    {
        throw ParamException("Invalid unsigned value - '" + param + "'");
    }

    if(val == ULONG_MAX && errno == ERANGE)
    {
        throw ParamException("Unsigned value out of range - '" + param + "'");
    }

    return val;
}



double doubleParam(std::string& params, const std::string& delimiters)
{
    std::string param = nextParam(params, delimiters);

    if(param.empty())
    {
        throw ParamException("Missing parameter");
    }

    char* endPtr;

    double val = strtod(param.c_str(), &endPtr);

    if(*endPtr)
    {
        throw ParamException("Invalid floating point value - '" + param + "'");
    }

    if((val == -HUGE_VAL || val == HUGE_VAL) && errno == ERANGE)
    {
        throw ParamException("Floating point value out of range - '" + param + "'");
    }

    return val;
}



int stringToInt(const std::string& string, int radix, const std::string& delimiters)
{
    std::string stringCopy(string);
    int val = intParam(stringCopy, radix, delimiters);
    if(stringCopy.size())
    {
        throw ParamException("stringToInt: Unexpected characters - '" + string + "'");
    }
    return val;
}


unsigned stringToUnsigned(const std::string& string, int radix, const std::string& delimiters)
{
    std::string stringCopy(string);
    unsigned val = unsignedParam(stringCopy, radix, delimiters);
    if(stringCopy.size())
    {
        throw ParamException("stringToUnsigned: Unexpected characters - '" + string + "'");
    }
    return val;
}


double stringToDouble(const std::string& string, const std::string& delimiters)
{
    std::string stringCopy(string);
    double val = doubleParam(stringCopy, delimiters);
    if(stringCopy.size())
    {
        throw ParamException("stringToDouble: Unexpected characters - '" + string + "'");
    }
    return val;
}


