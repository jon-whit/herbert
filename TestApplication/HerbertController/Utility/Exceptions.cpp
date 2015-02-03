///////////////////////////////////////////////////////
//
//  Exceptions.cpp
//
//  ITI Exceptions and Assert definitions
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#include <StdAfx.h>

#include <Exceptions.h>

#include <StackWalker.h>
#include <iostream>
#include <iomanip>


class ITIStackWalker : public StackWalker
{
public:
    ITIStackWalker() : StackWalker() {}
    ITIStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
    virtual void OnOutput(LPCSTR szText) { m_stack << szText; StackWalker::OnOutput(szText); }

    std::stringstream m_stack;
};




std::string ITI::getStackTrace()
{
    ITIStackWalker sw;
    sw.ShowCallstack();

    return sw.m_stack.str();
}



const char* ITI::getProcessorExceptionDescription(unsigned code)
{
    switch(code)
    {
    case EXCEPTION_ACCESS_VIOLATION:          return "Access Violation";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:        return "Integer Divide By Zero";
    case EXCEPTION_INT_OVERFLOW:              return "Integer Divide By Zero";
    case EXCEPTION_FLT_DENORMAL_OPERAND:      return "Floating Point Denormal Operation";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:        return "Floating Point Divide By Zero";
    case EXCEPTION_FLT_INEXACT_RESULT:        return "Floating Point Inexact Result";
    case EXCEPTION_FLT_INVALID_OPERATION:     return "Floating Point Invalid Operation";
    case EXCEPTION_FLT_OVERFLOW:              return "Floating Point Overflow";
    case EXCEPTION_FLT_STACK_CHECK:           return "Floating Point Stack Check";
    case EXCEPTION_FLT_UNDERFLOW:             return "Floating Point Underflow";
    case EXCEPTION_DATATYPE_MISALIGNMENT:     return "Datatype Misalignment";
    case EXCEPTION_BREAKPOINT:                return "Breakpoint";
    case EXCEPTION_SINGLE_STEP:               return "Single Step";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:     return "Array Bounds Exceeded";
    case EXCEPTION_PRIV_INSTRUCTION:          return "Privileged Instruction";
    case EXCEPTION_IN_PAGE_ERROR:             return "In Page Error";
    case EXCEPTION_ILLEGAL_INSTRUCTION:       return "Illegal Instruction";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:  return "Non-continuable Exception";
    case EXCEPTION_STACK_OVERFLOW:            return "Stack Overflow";
    case EXCEPTION_INVALID_DISPOSITION:       return "Invalid Disposition";
    case EXCEPTION_GUARD_PAGE:                return "Guard Page";
    case EXCEPTION_INVALID_HANDLE:            return "Invalid Handle";
    case CONTROL_C_EXIT:                      return "Control-C Exit";
    default:                                  return "Undefined Processor Exception";
    }
}





static void structuredExceptionTranslator(unsigned code, EXCEPTION_POINTERS *info)
{
    switch(code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_INVALID_DISPOSITION:
    case EXCEPTION_GUARD_PAGE:
    case EXCEPTION_INVALID_HANDLE:
        throw ITI::ProcessorException(code, *info);
        break;

    case CONTROL_C_EXIT:
        break;

    default:
        throw ITI::ProcessorException(code, *info);
        break;
    }
}




ITI::ProcessorException::ProcessorException(unsigned code, const EXCEPTION_POINTERS& info) throw()
    : m_code(code), Exception(getProcessorExceptionDescription(code))
{
    const EXCEPTION_RECORD& exception = *(info.ExceptionRecord);
    const CONTEXT&          context   = *(info.ContextRecord);

    m_address = exception.ExceptionAddress;

    ITIStackWalker sw;
    sw.GetNameFromAddr((DWORD64)m_address);



    std::stringstream s;

    s << std::setfill('0');
    s << "Processor Exception: (0x" << std::hex << code << ") " << getProcessorExceptionDescription(code) << std::endl
      << "  At: 0x" << std::hex << m_address << " " << sw.GetNameFromAddr((DWORD64)m_address) << std::endl
      << "  Context Record:" << std::endl
      << "    Flags:  0x" << std::setw(8) << context.ContextFlags << std::endl
      << "    Dr0:    0x" << std::setw(8) << context.Dr0 << std::endl
      << "    Dr1:    0x" << std::setw(8) << context.Dr1 << std::endl
      << "    Dr2:    0x" << std::setw(8) << context.Dr2 << std::endl
      << "    Dr3:    0x" << std::setw(8) << context.Dr3 << std::endl
      << "    Dr6:    0x" << std::setw(8) << context.Dr6 << std::endl
      << "    Dr7:    0x" << std::setw(8) << context.Dr7 << std::endl
      << "    FloatSave:" << std::endl
      << "      ControlWord:   0x" << std::setw(8) << context.FloatSave.ControlWord << std::endl
      << "      StatusWord:    0x" << std::setw(8) << context.FloatSave.StatusWord << std::endl
      << "      TagWord:       0x" << std::setw(8) << context.FloatSave.TagWord << std::endl
      << "      ErrorOffset:   0x" << std::setw(8) << context.FloatSave.ErrorOffset << std::endl
      << "      ErrorSelector: 0x" << std::setw(8) << context.FloatSave.ErrorSelector << std::endl
      << "      DataOffset:    0x" << std::setw(8) << context.FloatSave.DataOffset << std::endl
      << "      DataSelector:  0x" << std::setw(8) << context.FloatSave.DataSelector << std::endl
      << "      Cr0NpxState:   0x" << std::setw(8) << context.FloatSave.Cr0NpxState << std::endl
      << "    SegGs:  0x" << std::setw(8) << context.SegGs << std::endl
      << "    SegFs:  0x" << std::setw(8) << context.SegFs << std::endl
      << "    SegEs:  0x" << std::setw(8) << context.SegEs << std::endl
      << "    SegDs:  0x" << std::setw(8) << context.SegDs << std::endl
      << "    Edi:    0x" << std::setw(8) << context.Edi << std::endl
      << "    Esi:    0x" << std::setw(8) << context.Esi << std::endl
      << "    Ebx:    0x" << std::setw(8) << context.Ebx << std::endl
      << "    Edx:    0x" << std::setw(8) << context.Edx << std::endl
      << "    Ecx:    0x" << std::setw(8) << context.Ecx << std::endl
      << "    Eax:    0x" << std::setw(8) << context.Eax << std::endl
      << "    Ebp:    0x" << std::setw(8) << context.Ebp << std::endl
      << "    Eip:    0x" << std::setw(8) << context.Eip << std::endl
      << "    SegCs:  0x" << std::setw(8) << context.SegCs << std::endl
      << "    EFlags: 0x" << std::setw(8) << context.EFlags << std::endl
      << "    Esp:    0x" << std::setw(8) << context.Esp << std::endl
      << "    SegSs:  0x" << std::setw(8) << context.SegSs << std::endl;

    m_details = s.str();

    std::cout << s.str();
}


void ITI::ProcessorException::install() throw()
{
    _set_se_translator(structuredExceptionTranslator);
}



std::string convertWCharToString(const wchar_t* in)
{
    ITI_ASSERT_NOT_NULL(in);

    std::string s;

    while(*in)
    {
        char c = static_cast<char>(*in++);
        s += c;
    }

    return s;
}



void invalidParameterHandler(const wchar_t * expression,
                             const wchar_t * function,
                             const wchar_t * file,
                             unsigned int    line,
                             uintptr_t       pReserved)
{
    std::stringstream s;

    s << "Invalid Parameter Error:" << std::endl;

    if(expression)
    {
        s << "  Expression: " << convertWCharToString(expression) << std::endl;
    }

    if(function)
    {
        s << "  Function: " << convertWCharToString(function) << std::endl;
    }

    if(file)
    {
        s << "  File: " << convertWCharToString(file) << std::endl;
    }

    if(line)
    {
        s << "  Line: " << line << std::endl;
    }

    throw ITI::Exception(s.str());
}



void ITI::configureErrorHandlers()
{
    _set_invalid_parameter_handler(invalidParameterHandler);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
}