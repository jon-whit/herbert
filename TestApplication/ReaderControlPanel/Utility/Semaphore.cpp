///////////////////////////////////////////////////////////////////////////
//
//  Windows Semaphore Class
//
//  Copyright 2008
//  Developed by Brett Gilbert
//



#include <Semaphore.h>
#include <limits.h>
#include <assert.h>



///////////////////////////////////////////////////////////////
// Semaphore Class

Semaphore::Semaphore() :
    m_semHandle(CreateSemaphore(NULL, 0, INT_MAX, NULL))
{
    ASSERT(m_semHandle);
}


Semaphore::Semaphore(unsigned initialCount, unsigned maximumCount) :
    m_semHandle(CreateSemaphore(NULL, initialCount, maximumCount, NULL))
{
    ASSERT(m_semHandle);
}


    
Semaphore::~Semaphore()
{
    CloseHandle(m_semHandle);
}


void Semaphore::signal()
{
    if(!ReleaseSemaphore(m_semHandle, 1, NULL))
    {
        throw Exception("Max count exceeded");
    }
}



bool Semaphore::wait()
{
    return WaitForSingleObject(m_semHandle, INFINITE ) == WAIT_OBJECT_0;
}



bool Semaphore::wait(unsigned timeout_ms)
{
    return WaitForSingleObject(m_semHandle, (DWORD)timeout_ms) == WAIT_OBJECT_0;
}

    
    
// EOF




