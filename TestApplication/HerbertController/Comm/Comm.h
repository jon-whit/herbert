/////////////////////////////////////////////////////////////
//
//  Comm.h
//
//  PlateCycler Comm interface class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef Comm_h
#define Comm_h

#include <SerialEx.h>
#include <memory>
#include <list>
#include <afxwin.h>
#include <Mutex.h>
#include <CommPkt.h>
#include <Thread.h>
#include <ThreadData.h>
#include <ThreadQueue.h>
#include <Commands.h>
#include <AutoPointer.h>
#include <deque>
#include <Exceptions.h>



typedef ThreadQueue<Commands::Transaction::shared_ptr>                 TransactionThreadQueue;
typedef GuiMessagingThreadQueue<Commands::Transaction::shared_ptr>     TransactionGuiMessagingThreadQueue;

typedef ThreadQueue<Commands::PendingResponse::shared_ptr>             PendingResponseThreadQueue;
typedef GuiMessagingThreadQueue<Commands::PendingResponse::shared_ptr> PendingResponseGuiMessagingThreadQueue;

typedef ThreadQueue<Commands::InfoResponse::shared_ptr>                InfoResponseThreadQueue;
typedef GuiMessagingThreadQueue<Commands::InfoResponse::shared_ptr>    InfoResponseGuiMessagingThreadQueue;

typedef ThreadQueue<Commands::ErrorMessage::shared_ptr>                ErrorMessageThreadQueue;
typedef GuiMessagingThreadQueue<Commands::ErrorMessage::shared_ptr>    ErrorMessageGuiMessagingThreadQueue;

typedef ThreadQueue<Commands::LogMessage::shared_ptr>                  LogMessageThreadQueue;
typedef GuiMessagingThreadQueue<Commands::LogMessage::shared_ptr>      LogMessageGuiMessagingThreadQueue;

typedef ThreadQueue<CommPkt::shared_ptr>                               MonitorThreadQueue;
typedef GuiMessagingThreadQueue<CommPkt::shared_ptr>                   MonitorGuiMessagingThreadQueue;




class Comm : public CSerialEx
{
public:
    // Public Types
    class Exception : public ITI::Exception
    {
        public:
            inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };

public:
    // Public API
    ~Comm();

    BOOL Open(const std::string& port);
    LONG Close();

    void setErrorQueue(ErrorMessageThreadQueue::shared_ptr errorMessageQueue);
    void setLogQueue(LogMessageThreadQueue::shared_ptr logMessageQueue);
    void setMonitorQueue(MonitorThreadQueue::shared_ptr m_monitorQueue);

    void sendCommand(TransactionThreadQueue::shared_ptr     rspQueue,
                     Commands::Transaction::shared_ptr      transaction);

    void sendCommand(PendingResponseThreadQueue::shared_ptr pendingRspQueue,
                     TransactionThreadQueue::shared_ptr     rspQueue,
                     Commands::Transaction::shared_ptr      transaction);

    void sendCommand(InfoResponseThreadQueue::shared_ptr    infoRspQueue,
                     PendingResponseThreadQueue::shared_ptr pendingRspQueue,
                     TransactionThreadQueue::shared_ptr     rspQueue,
                     Commands::Transaction::shared_ptr      transaction);

protected:
    Comm(CSerial::EBaudrate baudRate);

private:
    // Explicitly disallow operator=
    Comm& operator=(const Comm&);


private:
    // Private Types
    enum Constants
    {
        TRANSACTION_WAIT_TIME_ms        = 1,
        RSP_TIMEOUT_ms                  = 1000,
        TX_TIMEOUT_ms                   = 200,
        EVENT_WAIT_TIME_ms              = 1,
        WAIT_FOR_THREAD_EXIT_TIMEOUT_ms = 1000,
        MAX_RETRY_COUNT                 = 3
    };

    class TransactionContainer
    {
    public:
        enum { MAX_WAIT_TIME_ms = 1000 };

        typedef boost::shared_ptr<TransactionContainer> shared_ptr;
        
        TransactionContainer(Commands::Transaction::shared_ptr      transaction,
                             TransactionThreadQueue::shared_ptr     rspQueue,
                             PendingResponseThreadQueue::shared_ptr pendingRspQueue,
                             InfoResponseThreadQueue::shared_ptr    infoRspQueue)
            : m_transaction(transaction), m_rspQueue(rspQueue)
            , m_pendingRspQueue(pendingRspQueue), m_infoRspQueue(infoRspQueue)
        {
            ITI_ASSERT_NOT_NULL(transaction);
            ResetWaitTime();
        };

        unsigned GetWaitingTime() {return GetTickCount() - m_startTime_ms;}

        void ResetWaitTime()
        {
            m_startTime_ms = GetTickCount();
        }

        Commands::Transaction::shared_ptr      m_transaction;
        TransactionThreadQueue::shared_ptr     m_rspQueue;
        PendingResponseThreadQueue::shared_ptr m_pendingRspQueue;
        InfoResponseThreadQueue::shared_ptr    m_infoRspQueue;

    private:
        TransactionContainer();
        TransactionContainer& operator=(const TransactionContainer&);

        unsigned m_startTime_ms;
    };


private:
    // Private Member Functions
    static DWORD WINAPI  startCommThread(void* commObject);
    DWORD                commThread();

    static DWORD WINAPI  startPendingThread(void* commObject);
    DWORD                pendingThread();

    static DWORD WINAPI  startLoopbackThread(void* commObject);
    DWORD                loopbackThread();

    void                 killCommThreads();

    virtual void         OnEvent (EEvent eEvent, EError eError);
    void                 ProcessRxPkt(const CString& rxStr);


private:
    // Member Data
    CSerial::EBaudrate      m_baudRate;

    HANDLE                  m_commThreadHandle;
    DWORD                   m_commThreadId;
    Thread::State           m_commThreadState;

    HANDLE                  m_pendingThreadHandle;
    DWORD                   m_pendingThreadId;
    Thread::State           m_pendingThreadState;

    HANDLE                  m_loopbackThreadHandle;
    DWORD                   m_loopbackThreadId;
    Thread::State           m_loopbackThreadState;

    CString                 m_rxBuffer;

    ThreadQueue<CommPkt::shared_ptr>               m_rxQueue;
    ThreadQueue<CommPkt::shared_ptr>               m_unexpectedRspQueue;
    ThreadQueue<CommPkt::shared_ptr>               m_unhandledRspQueue;
    ThreadQueue<CommPkt::shared_ptr>               m_loopbackTxQueue;
    
    ThreadQueue<TransactionContainer::shared_ptr>  m_sendTransactionQueue;
    ThreadQueue<TransactionContainer::shared_ptr>  m_pendingTransactionQueue;

    Mutex                                          m_messageQueueMutex;
    ErrorMessageThreadQueue::shared_ptr            m_errorMessageQueue;
    LogMessageThreadQueue::shared_ptr              m_logMessageQueue;
    MonitorThreadQueue::shared_ptr                 m_monitorQueue;
};

#endif
