/////////////////////////////////////////////////////////////
//
//  Logging.cpp
//
//  Logging server
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#include "StdAfx.h"
#include <Logging.h>
#include <AutoFilePointer.h>
//#include <Configuration.h>
#include <ThreadQueue.h>
#include <Thread.h>
#include <boost/scoped_ptr.hpp>
#include <Timestamp.h>
//#include <UnsolicitedMessageServer.h>
#include <io.h>

//#include <AppData.h>



const char DEFAULT_LOG_FILE_NAME[] = "ReaderControlPanel.log";


const int MAX_LOG_LINE_LEN              = 5000;
const int MAX_LOG_ROTATE_FILENAME_COUNT = 10;
const int MAX_LOG_FILE_SIZE             = 10000000;


DWORD dwTlsIndex;


class Logging
{
public:
    static Logging& instance()  { static Logging l; return l; }
    ~Logging()                  { killLoggingThread(); }

    void startLoggingServer();

    void logMessage(const std::string& str);


private:
    Logging();
    Logging(const Logging& ch);
    Logging& operator=(const Logging& ch);



private:
    enum Constants
    {
        THREAD_MSG_WAIT_TIME_ms         = 10,
        WAIT_FOR_THREAD_EXIT_TIMEOUT_ms = 500
    };

    class LogMsg
    {
    public:
        LogMsg(const std::string& msg) : m_msg(msg) {}

        typedef boost::shared_ptr<LogMsg> shared_ptr;

        Timestamp   m_timestamp;
        std::string m_msg;
    private:
        LogMsg();
    };


private:
    static DWORD WINAPI startLoggingThread(void* loggingObject);
    void                killLoggingThread();
    DWORD               loggingThread();
    FILE*               openLogFile();
    void                rotateLogFiles();

    Thread::State       m_threadState;
    HANDLE              m_threadHandle;
    DWORD               m_threadId;
    const std::string   m_filename;
    int                 m_maxLogFileSize;

    ThreadQueue<LogMsg::shared_ptr> m_messageQueue;
};




Logging& logging = Logging::instance();



Logging::Logging()
: m_filename(std::string(DEFAULT_LOG_FILE_NAME)),
  m_maxLogFileSize(MAX_LOG_FILE_SIZE)
{
    startLoggingServer();
}



void Logging::startLoggingServer()
{
    if(m_threadState.start())
    {
        m_threadHandle = CreateThread(NULL, 0, startLoggingThread, this, 0, &m_threadId);
    }
}



DWORD WINAPI Logging::startLoggingThread(void* loggingObject)
{
    ITI::ProcessorException::install();

    std::stringstream errMsg;

    try
    {
        return ((Logging*)loggingObject)->loggingThread();
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error in Logging Server:" << std::endl
               << ex.what() << std::endl << "Logging Server Thread exited";
    }
    catch(ITI::ProcessorException& ex)
    {
        errMsg << "Processor Exception in Logging Server:" << std::endl
               << ex.details() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Logging Server Thread exited";
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception in Logging Server:" << std::endl
               << ex.what() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Logging Server Thread exited";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception in Logging Server:" << std::endl
               << ex.what() << std::endl << "Logging Server Thread exited";
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception in Logging Server"
               << std::endl << "Logging Server Thread exited";
    }


    ((Logging*)loggingObject)->m_threadState = Thread::State::finished;

    ::MessageBox(NULL, errMsg.str().c_str(), "Logging Thread Error", MB_ICONEXCLAMATION | MB_OK);
    return -1;
}


void Logging::killLoggingThread()
{
    if(m_threadState.stop())
    {
        if(WaitForSingleObject(m_threadHandle, WAIT_FOR_THREAD_EXIT_TIMEOUT_ms) != WAIT_OBJECT_0)
        {
            //Thread did not die in time - kill it
            TerminateThread(m_threadHandle, -1);

            printf("Killed Command Handler thread\n");
        }
    }
}



DWORD Logging::loggingThread()
{
    m_threadState = Thread::State::running;

    try
    {
        while(m_threadState.isRunning())
        {
            AutoFilePointer fp = openLogFile();

            while (m_threadState.isRunning())
            {
                LogMsg::shared_ptr msg = m_messageQueue.dequeue(THREAD_MSG_WAIT_TIME_ms);

                if(msg)
                {
                    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d: %s\n",
                        msg->m_timestamp.m_year,
                        msg->m_timestamp.m_month,
                        msg->m_timestamp.m_day,
                        msg->m_timestamp.m_hour,
                        msg->m_timestamp.m_minute,
                        msg->m_timestamp.m_second,
                        msg->m_msg.c_str());
                    fflush(fp);

                    if (ftell(fp) >= m_maxLogFileSize) {
                        fclose(fp);
                        rotateLogFiles();
                        fp = openLogFile();
                    }
                }
            }
        }
    }
    catch(Thread::Exit)
    {
        // Exiting thread - no need to handle further
    }

    m_threadState = Thread::State::finished;
    return 0;
}



void Logging::logMessage(const std::string& str)
{
    LogMsg::shared_ptr logMsg(new LogMsg(str));
    m_messageQueue.enqueue(logMsg);
}



FILE* Logging::openLogFile()
{
    FILE *file = fopen(m_filename.c_str(), "a");

    if (!file)
    {
        throw ITI::Exception("Failed to open log file");
    }

    return file;
}



void Logging::rotateLogFiles()
{
    std::ostringstream filename;

    for (int i = MAX_LOG_ROTATE_FILENAME_COUNT; i > 0; --i)
    {
        filename.str("");
        filename << m_filename << i;

        if (_access(filename.str().c_str(), 0) != -1)
        {
            if (i < MAX_LOG_ROTATE_FILENAME_COUNT)
            {
                std::ostringstream newFilename;
                
                newFilename << m_filename << (i + 1);
                rename(filename.str().c_str(), newFilename.str().c_str());
            }
            else
            {
                remove(filename.str().c_str());
            }
        }
    }

    filename.str("");
    filename << m_filename << 1;
    rename(m_filename.c_str(), filename.str().c_str());
}



int log(const std::string& str)
{
    Logging::instance().logMessage(str);
    return str.size();
}



int log(const char* format, ...)
{
    std::string s;
    
    s.resize(MAX_LOG_LINE_LEN + 1);

    va_list argList;

    va_start(argList, format);
    int rc = _vsnprintf(&s.at(0), MAX_LOG_LINE_LEN, format, argList);
    va_end(argList);

    if(rc < 0)
    {
        rc = MAX_LOG_LINE_LEN;
    }

    s.resize(rc);

    Logging::instance().logMessage(s);

    return rc;
}
