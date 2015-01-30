///////////////////////////////////////////////////////////////////////////
//
//  Windows Mutex Class
//
//  Copyright 2008
//  Developed by Brett Gilbert
//


#ifndef Mutex_h
#define Mutex_h


#include <Windows.h>



class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    // Explicitly disallow operator=
    Mutex& operator=(const Mutex&);

    CRITICAL_SECTION m_criticalSection;
};



class MutexLock
{
public:
    MutexLock(Mutex& mutex) : m_mutex(mutex) {m_mutex.lock();}
    ~MutexLock()                             {m_mutex.unlock();}
private:
    // Explicitly disallow default constructor and operator=
    MutexLock();
    MutexLock& operator=(const MutexLock&);

    Mutex& m_mutex;
};




#endif
