///////////////////////////////////////////////////////////////////////////
//
//  Windows Semaphore Class
//
//  Copyright 2008
//  Developed by Brett Gilbert
//


#ifndef Semaphore_h
#define Semaphore_h


#include <Exceptions.h>
#include <stdexcept>
#include <string>
#include <Mutex.h>
#include <Windows.h>


class Semaphore
{
public:
    class Exception : public ITI::Exception
    {
        public:
            inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };



public:
    Semaphore();
    Semaphore::Semaphore(unsigned initialCount, unsigned maximumCount);

    ~Semaphore();


    void signal();
    bool wait();
    bool wait(unsigned timeout_ms);

private:
    // Explicitly disallow operator=
    Semaphore& operator=(const Semaphore&);

    Mutex    m_mutex;
    HANDLE   m_semHandle;
};


#endif
