///////////////////////////////////////////////////////
//
//  Exceptions.h
//
//  ITI Exceptions and Assert definitions
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#ifndef Exceptions_h
#define Exceptions_h


#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

#include <afxwin.h>

namespace ITI
{
    std::string getStackTrace();
    void configureErrorHandlers();


    /////////////////////////////////////////////
    // Exceptions

    class Exception : public std::runtime_error
    {
    public:
        Exception(const std::string& ex) : std::runtime_error(ex), m_stackTrace(getStackTrace()) {}
        const char* stackTrace() {return m_stackTrace.c_str();}
    protected:
        std::string m_stackTrace;
    };


    
    class Assert : public Exception
    {
    public:
        Assert(const std::string& ex) : Exception(ex)
        {
            std::stringstream s;

            s << Exception::what() << std::endl;
            s << "Stack Trace:" << std::endl;
            s << m_stackTrace;

            m_what = s.str();
        }
        virtual const char* what() {return m_what.c_str();}
    private:
        std::string m_what;
    };



    class ProcessorException : public Exception
    {
        public:
            ProcessorException(unsigned code, const EXCEPTION_POINTERS& info) throw();

            static void install() throw();

            unsigned    code()    { return m_code; }
            const void* where()   { return m_address; }
            const char* details() { return m_details.c_str(); }

        private:
            unsigned    m_code;
            const void* m_address;
            std::string m_details;
    };


    
    /////////////////////////////////////////////
    // Helper and Exception implementation functions

    const char* getProcessorExceptionDescription(unsigned code);


    inline void error(const char* desc, const char* file, int line)
    {
        std::stringstream s;
        s << "Error: '" << desc << "' - " << file << "@" << line << std::endl;

        throw ITI::Assert(s.str());
    }

    template <typename T>
    inline void assertTrue(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(!expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' not true - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T>
    inline void assertFalse(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' (";
            s << ((sizeof(expr) == 1) ? (unsigned)expr : expr) << " not false - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T>
    inline void assertZero(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' (";
            s << ((sizeof(expr) == 1) ? (unsigned)expr : expr) << " not zero - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T>
    inline void assertNotZero(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(!expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' zero - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T>
    inline void assertNull(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' (";
            s << ((sizeof(expr) == 1) ? (unsigned)expr : expr) << " not null - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T>
    inline void assertNotNull(const T& expr, const char* expr_text, const char* file, int line)
    {
        if(!expr)
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr_text << "' null - " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertEqual(const T& expr1, const U& expr2,
                            const char* expr1_text, const char* expr2_text,
                            const char* file, int line)
    {
        if(!(expr1 == expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " == " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " == ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertNotEqual(const T& expr1, const U& expr2,
                               const char* expr1_text, const char* expr2_text,
                               const char* file, int line)
    {
        if((expr1 == expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " != " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " != ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertLess(const T& expr1, const U& expr2,
                           const char* expr1_text, const char* expr2_text,
                           const char* file, int line)
    {
        if(!(expr1 < expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " < " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " < ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertLessEqual(const T& expr1, const U& expr2,
                                const char* expr1_text, const char* expr2_text,
                                const char* file, int line)
    {
        if(!(expr1 <= expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " <= " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " <= ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertGreater(const T& expr1, const U& expr2,
                              const char* expr1_text, const char* expr2_text,
                              const char* file, int line)
    {
        if(!(expr1 > expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " > " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " > ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

    template <typename T, typename U>
    inline void assertGreaterEqual(const T& expr1, const U& expr2,
                                   const char* expr1_text, const char* expr2_text,
                                   const char* file, int line)
    {
        if(!(expr1 >= expr2))
        {
            std::stringstream s;
            s << "Assertion failure: '" << expr1_text << " >= " << expr2_text << "' (";
            s << ((sizeof(expr1) == 1) ? (unsigned)expr1 : expr1) << " >= ";
            s << ((sizeof(expr2) == 1) ? (unsigned)expr2 : expr2) << ")" << std::endl;
            s << "  " << file << "@" << line << std::endl;

            throw ITI::Assert(s.str());
        }
    }

}


/////////////////////////////////////////////
// Exception Macros

#define ITI_ASSERT(expr)                       ITI::assertTrue(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_TRUE(expr)                  ITI::assertTrue(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_FALSE(expr)                 ITI::assertFalse(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_ZERO(expr)                  ITI::assertZero(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_NOT_ZERO(expr)              ITI::assertNotZero(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_NULL(expr)                  ITI::assertNull(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_NOT_NULL(expr)              ITI::assertNotNull(expr, #expr, __FILE__, __LINE__)
#define ITI_ASSERT_EQUAL(expr1, expr2)         ITI::assertEqual(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)
#define ITI_ASSERT_NOT_EQUAL(expr1, expr2)     ITI::assertNotEqual(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)
#define ITI_ASSERT_LESS(expr1, expr2)          ITI::assertLess(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)
#define ITI_ASSERT_LESS_EQUAL(expr1, expr2)    ITI::assertLessEqual(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)
#define ITI_ASSERT_GREATER(expr1, expr2)       ITI::assertGreater(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)
#define ITI_ASSERT_GREATER_EQUAL(expr1, expr2) ITI::assertGreaterEqual(expr1, expr2, #expr1, #expr2, __FILE__, __LINE__)

#define ITI_ERROR(desc)                        ITI::error(desc, __FILE__, __LINE__)


#endif
