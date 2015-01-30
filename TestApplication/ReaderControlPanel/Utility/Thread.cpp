#include <Thread.h>
#include <process.h>

Thread::Thread(const std::string& theadName) :
    m_threadState(notStarted),
    m_threadName(theadName),
    m_threadHandle(0)
{
}


Thread::~Thread()
{
    stop();
    waitForStop(0);
    if(m_threadHandle)
    {
        CloseHandle(m_threadHandle);
    }
}



void Thread::start(unsigned stackSize)
{
    {
        MutexLock lock(m_threadStateMutex);

        if(!(m_threadState == notStarted) && !(m_threadState == finished))
        {
            std::stringstream s;
            s << "Can't start thread '" << m_threadName << "' - alread running";
            throw Exception(s.str());
        }

        if(m_threadHandle)
        {
            CloseHandle(m_threadHandle);
        }

        m_threadState = starting;
    }

    m_threadHandle = (HANDLE)_beginthreadex(NULL, stackSize, threadEntry, this, 0, &m_threadId);
}


void Thread::stop()
{
    MutexLock lock(m_threadStateMutex);
    if(m_threadState != notStarted && m_threadState != finished)
    {
        m_threadState = stopping;
    }
}


void Thread::waitForStop(unsigned maxWaitTime)
{
    if(isActive())
    {
        if(WaitForSingleObject(m_threadHandle, maxWaitTime) != WAIT_OBJECT_0)
        {
            //Thread did not die in time - kill it
            TerminateThread(m_threadHandle, -1);

            TRACE("Killed thread '%s'\n", m_threadName.c_str());
        }
    }
}



unsigned WINAPI Thread::threadEntry(void* threadObject)
{
    ITI::ProcessorException::install();

    Thread* thread = static_cast<Thread*>(threadObject);

    std::stringstream errMsg;

    try
    {
        bool start = false;

        {
            MutexLock lock(thread->m_threadStateMutex);
            if(thread->m_threadState == starting)
            {
                thread->m_threadState = running;
                start = true;
            }
        }

        if(start)
        {
            TRACE("Thread '%s' started\n", thread->m_threadName.c_str());
            DWORD rc = thread->thread();
            TRACE("Thread '%s' finished\n", thread->m_threadName.c_str());

            MutexLock lock(thread->m_threadStateMutex);
            thread->m_threadState = finished;

            return rc;
        }

        errMsg << "Failed to start thread";
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error:" << std::endl
               << ex.what();
    }
    catch(ITI::ProcessorException& ex)
    {
        errMsg << "Unhandled exception:" << std::endl
               << ex.what() << std::endl
               << ex.details() << std::endl
               << "Stack Trace:" << std::endl
               << ex.stackTrace();
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception:" << std::endl
               << ex.what() << std::endl
               << "Stack Trace:" << std::endl
               << ex.stackTrace();
    }
    catch(Exit&)
    {
        errMsg << "Unhandled thread exit exception";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception:" << std::endl
               << ex.what();
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception";
    }

    thread->m_threadState = finished;

    std::stringstream msg;

    msg << "Thread '" << thread->m_threadName << "' error:" << std::endl << errMsg.str() << std::endl << "Thread exited";

    thread->handleException(msg.str());

    return -1;

}

void Thread::handleException(const std::string& msg)
{
    TRACE("Thread '%s' error: %s\n", m_threadName.c_str(), msg.c_str());
}


void Thread::threadSleep(DWORD sleepTime_ms)
{
    DWORD startTime_ms = GetTickCount();
    DWORD currentTime_ms;

    while(!exitSleep() &&
          (currentTime_ms = GetTickCount()) - startTime_ms < sleepTime_ms)
    {
        Sleep(1);
    }
}
