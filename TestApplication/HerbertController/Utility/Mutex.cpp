///////////////////////////////////////////////////////////////////////////
//
//  Windows Mutex Class
//
//  Copyright 2008
//  Developed by Brett Gilbert
//



#include <Mutex.h>



///////////////////////////////////////////////////////////////
// Mutex Class

Mutex::Mutex()
{
    InitializeCriticalSection(&m_criticalSection);
}

Mutex::~Mutex()
{
    DeleteCriticalSection(&m_criticalSection);
}

void Mutex::lock()
{
    EnterCriticalSection(&m_criticalSection);
}

void Mutex::unlock()
{
    LeaveCriticalSection(&m_criticalSection);
}


// EOF
