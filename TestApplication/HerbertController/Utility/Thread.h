////////////////////////////////////////////////////////////////
//
//  Thread.h
//
//  Thread Utilities
//
//  Copyright 2009
//  Created by Brett C. Gilbert

#ifndef Thread_h
#define Thread_h

#include <afxwin.h>
#include <Mutex.h>
#include <Exceptions.h>


class Thread
{
public:
    // TODO: Legacy code.  Make private

    class Exit  // Unique object to throw and exit a thread
    {
    };

    class State
    {
    public:
        enum States
        {
            notStarted,
            starting,
            running,
            stopping,
            finished
        };

        State()             : m_state(notStarted) {}
        State(States state) : m_state(state)      {}
        ~State() {}

        inline States getState()               { MutexLock lock(m_mutex); return m_state; }
        inline void   setState(States state)   { MutexLock lock(m_mutex); m_state = state; }

        inline State& operator=(States state)  { setState(state); return *this; }
        inline bool   operator==(States state) { MutexLock lock(m_mutex); return m_state == state; }

        inline bool start()
            {
                MutexLock lock(m_mutex);
                if(m_state == notStarted || m_state == finished)
                {
                    m_state = starting;
                    return true;
                }
                else
                    return false;
            }
        inline bool stop()      
            {
                MutexLock lock(m_mutex);
                if(m_state != notStarted && m_state != finished)
                {
                    m_state = stopping;
                    return true;
                }
                else
                    return false;
            }
        inline bool isActive()   { MutexLock lock(m_mutex); return m_state != notStarted && m_state != finished; }
        inline bool isRunning()  { MutexLock lock(m_mutex); return m_state == running; }
        inline bool isFinished() { MutexLock lock(m_mutex); return m_state == finished; }

    private:
        States m_state;
        Mutex  m_mutex;
    };

    // END TODO: Legacy code.



public:
    class Exception : public ITI::Exception
    {
    public:
        inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };

    enum States
    {
        notStarted,
        starting,
        running,
        stopping,
        finished
    };

    enum Constants
    {
        DEFAULT_MAX_THEAD_EXIT_WAITTIME_ms = 500,
    };

    Thread(const std::string& theadName);
    ~Thread();

    virtual void start(unsigned stackSize = 0);
    virtual void stop();
    virtual void waitForStop(unsigned maxWaitTime = DEFAULT_MAX_THEAD_EXIT_WAITTIME_ms);

    States state()      { MutexLock lock(m_threadStateMutex); return m_threadState; }
    bool   isActive()   { MutexLock lock(m_threadStateMutex); return m_threadState != notStarted && m_threadState != finished; }
    bool   isRunning()  { MutexLock lock(m_threadStateMutex); return m_threadState == running; }
    bool   isFinished() { MutexLock lock(m_threadStateMutex); return m_threadState == finished; }

    const std::string& name() { return m_threadName; }

protected:
    // Disallow
    Thread();
    Thread(const Thread&);
    Thread& operator=(const Thread&);

    static unsigned WINAPI threadEntry(void* threadObject);
    virtual DWORD       thread() = 0;
    virtual void        threadSleep(DWORD sleepTime_ms);
    virtual bool        exitSleep()  { return !isRunning(); }
    virtual bool        exitThread() { return !isRunning(); }
    virtual void        handleException(const std::string& msg);

    std::string         m_threadName;
    HANDLE              m_threadHandle;
    unsigned            m_threadId;
    States              m_threadState;
    Mutex               m_threadStateMutex;
};

#endif
