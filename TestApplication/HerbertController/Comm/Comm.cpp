/////////////////////////////////////////////////////////////
//
//  Comm.cpp
//
//  PlateCycler Comm interface class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <Comm.h>
#include <Mutex.h>
#include <StringFormatting.h>



#define USE_LOOPBACK 0



Comm::Comm(CSerial::EBaudrate baudRate) :
    m_commThreadHandle(0),
    m_commThreadId(0),
    m_baudRate(baudRate)
{
}



Comm::~Comm()
{
}



BOOL Comm::Open(const std::string& port)
{
    killCommThreads();

    if(IsOpen()) CSerialEx::Close();

    std::string portStr = "\\\\.\\" + port;
    int status = CSerialEx::Open(portStr.c_str(), 0, 0, true);
    
    if(status != ERROR_SUCCESS || !IsOpen())
    {
        return false;
    }

    SetMask();
    Setup(m_baudRate);
    Purge();

    if(m_commThreadState.start())
    {
        m_commThreadHandle = CreateThread(NULL, 0, startCommThread, this, 0, &m_commThreadId);
    }

    if(m_pendingThreadState.start())
    {
        m_pendingThreadHandle = CreateThread(NULL, 0, startPendingThread, this, 0, &m_pendingThreadId);
    }

    #if USE_LOOPBACK
        if(m_loopbackThreadState.start())
        {
            m_loopbackThreadHandle = CreateThread(NULL, 0, startLoopbackThread, this, 0, &m_loopbackThreadId);
        }
    #endif

    return true;
}



LONG Comm::Close()
{
    killCommThreads();
    return CSerialEx::Close();
}



void Comm::setErrorQueue(ErrorMessageThreadQueue::shared_ptr errorMessageQueue)
{
    MutexLock lock(m_messageQueueMutex);
    m_errorMessageQueue = errorMessageQueue;
}



void Comm::setLogQueue(LogMessageThreadQueue::shared_ptr logMessageQueue)
{
    MutexLock lock(m_messageQueueMutex);
    m_logMessageQueue = logMessageQueue;
}



void Comm::setMonitorQueue(MonitorThreadQueue::shared_ptr monitorQueue)
{
    MutexLock lock(m_messageQueueMutex);
    m_monitorQueue = monitorQueue;
}



void Comm::sendCommand(TransactionThreadQueue::shared_ptr rspQueue, Commands::Transaction::shared_ptr transaction)
{
    PendingResponseThreadQueue::shared_ptr unusedPendingRspQueue;
    sendCommand(unusedPendingRspQueue, rspQueue, transaction);
}


void Comm::sendCommand(PendingResponseThreadQueue::shared_ptr pendingRspQueue,
                       TransactionThreadQueue::shared_ptr     rspQueue,
                       Commands::Transaction::shared_ptr      transaction)
{
    InfoResponseThreadQueue::shared_ptr unusedInfoRspQueue;
    sendCommand(unusedInfoRspQueue, pendingRspQueue, rspQueue, transaction);
}



void Comm::sendCommand(InfoResponseThreadQueue::shared_ptr    infoRspQueue,
                       PendingResponseThreadQueue::shared_ptr pendingRspQueue,
                       TransactionThreadQueue::shared_ptr     rspQueue,
                       Commands::Transaction::shared_ptr      transaction)
{
    if(!IsOpen())
    {
        throw Exception("Comm Port not open!");
    }

    transaction->cmdPktString(); // Verifies pkt

    TransactionContainer::shared_ptr tc(new TransactionContainer(transaction, rspQueue, pendingRspQueue, infoRspQueue));
    m_sendTransactionQueue.enqueue(tc);
}

                     
DWORD WINAPI Comm::startCommThread(void* commObject)
{
    ITI::ProcessorException::install();

    std::stringstream errMsg;

    try
    {
        return ((Comm*)commObject)->commThread();
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error in Comm thread:" << std::endl
               << ex.what() << std::endl << "Comm Thread exited";
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception in Comm thread:" << std::endl
               << ex.what() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Comm Thread exited";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception in Comm thread:" << std::endl
               << ex.what() << std::endl << "Comm Thread exited";
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception in Comm thread"
               << std::endl << "Comm Thread exited";
    }

    ::MessageBox(NULL, errMsg.str().c_str(), "Comm Thread Error", MB_ICONEXCLAMATION | MB_OK);

    return -1;
}



DWORD Comm::commThread()
{
    m_commThreadState = Thread::State::running;

    TRACE("COMM Thread running\n");

    int count = 0;

    while(m_commThreadState.isRunning())
    {
        TransactionContainer::shared_ptr container = m_sendTransactionQueue.dequeue(TRANSACTION_WAIT_TIME_ms);

        if(container)
        {
            bool                                   transactionHandled = false;

            CommPkt::shared_ptr                    rxPkt;

            PendingResponseThreadQueue::shared_ptr pendingRspQueue = container->m_pendingRspQueue;
            TransactionThreadQueue::shared_ptr     rspQueue        = container->m_rspQueue;
            Commands::Transaction::shared_ptr      transaction     = container->m_transaction;

            int retryCount = 0;

            try
            {
                bool  rspReceived = false;
                DWORD status      = ERROR_SUCCESS;

                while(!rspReceived && retryCount < MAX_RETRY_COUNT && m_commThreadState.isRunning())
                {
                    retryCount++;

                    #if USE_LOOPBACK
                        CommPkt::shared_ptr loopbackCmd(new CommPkt(transaction->cmdPktString()));
                        m_loopbackTxQueue.enqueue(loopbackCmd);
                        status = ERROR_SUCCESS;
                    #else
                        status = Write(transaction->cmdPktString(), NULL, NULL, TX_TIMEOUT_ms);
                    #endif

                    if(status != ERROR_SUCCESS)
                    {
                        if(status == ERROR_IO_INCOMPLETE)
                        {
                            // Overlapped IO did not complete - Sleep and wait for rsp
                            Sleep(1);
                        }
                        else
                        {
                            continue;
                        }
                    }

                    unsigned startTime_ms = GetTickCount();

                    while((GetTickCount() - startTime_ms < RSP_TIMEOUT_ms) && m_commThreadState.isRunning())
                    {
                        rxPkt = m_rxQueue.dequeue(1);

                        if(rxPkt)
                        {
                            //TRACE("RxPkt %s\n", rxPkt->GetFullPktStr());

                            if(transaction->validateRsp(rxPkt.get()))
                            {
                                rspReceived = true;

                                if(rxPkt->GetStatus() == COMM_STATUS_PENDING)
                                {
                                    //TRACE("Pending rsp %s\n", transaction->rspPktString());
                                    if(pendingRspQueue)
                                    {
                                        Commands::PendingResponse::shared_ptr pendingResponse(new
                                            Commands::PendingResponse(transaction, rxPkt));
                                        pendingRspQueue->enqueue(pendingResponse);
                                    }

                                    container->ResetWaitTime();
                                    m_pendingTransactionQueue.enqueue(container);

                                    transactionHandled = true;
                                }

                                break;
                            }
                            else
                            {
                                m_unhandledRspQueue.enqueue(rxPkt);
                            }
                        }
                    }
                }

                if(status != ERROR_SUCCESS)
                {
                    CString str;
                    str.Format("Comm TX failed, status = %d", status);
                    transaction->setErrorString(str);
                }
                else if(!rspReceived && m_commThreadState.isRunning())
                {
                    transaction->setErrorString("No response from target");
                }
            }
            catch(CommPkt::Exception& ex)
            {
                transaction->setErrorString(ex.what());
            }
            catch(std::runtime_error& ex)
            {
                transaction->setErrorString(ex.what());
            }
            catch(...)
            {
                transaction->setErrorString("Comm Error");
            }

            if(!transactionHandled)
            {
                //TRACE("Rsp %s\n", transaction->rspPktString());
                rspQueue->enqueue(transaction);
            }
        }
        else
        {
            CommPkt::shared_ptr rxPkt(m_rxQueue.dequeue(1));

            if(rxPkt)
            {
                //TRACE("RX Unhandled Rsp %s\n", rxPkt->GetFullPktStr());
                m_unhandledRspQueue.enqueue(rxPkt);
            }
        }
    }

    m_commThreadState = Thread::State::finished;

    TRACE("COMM Thread exiting\n");
    return 0;
}



DWORD WINAPI Comm::startPendingThread(void* commObject)
{
    ITI::ProcessorException::install();

    std::stringstream errMsg;

    try
    {
        return ((Comm*)commObject)->pendingThread();
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error in Comm Pending thread:" << std::endl
               << ex.what() << std::endl << "Comm Pending Thread exited";
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception in Comm Pending thread:" << std::endl
               << ex.what() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Comm Pending Thread exited";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception in Comm Pending thread:" << std::endl
               << ex.what() << std::endl << "Comm Pending Thread exited";
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception in Comm Pending thread"
               << std::endl << "Comm Thread exited";
    }

    ::MessageBox(NULL, errMsg.str().c_str(), "Comm Pending Thread Error", MB_ICONEXCLAMATION | MB_OK);

    return -1;
}


DWORD Comm::pendingThread()
{
    std::deque<TransactionContainer::shared_ptr> pendingTransactions;

    m_pendingThreadState = Thread::State::running;

    TRACE("Pending Thread running\n");

    while(m_pendingThreadState.isRunning())
    {
        CommPkt::shared_ptr unhandledRsp = m_unhandledRspQueue.dequeue(0);


        // Check for transaction after unhandled response so as not to miss the transaction
        TransactionContainer::shared_ptr pendingTransaction =
            m_pendingTransactionQueue.dequeue(unhandledRsp ? 0 : TRANSACTION_WAIT_TIME_ms);
        if(pendingTransaction)
        {
            //TRACE("Pending Transaction %s\n", pendingTransaction->m_transaction->cmdPktString());
            pendingTransaction->m_transaction->setOfflineRspFlag();
            pendingTransactions.push_back(pendingTransaction);
        }


        if(unhandledRsp)
        {
            if(Commands::Transaction::isErrorMessage(unhandledRsp.get()))
            {
                MutexLock lock(m_messageQueueMutex);

                if(m_errorMessageQueue)
                {
                    Commands::ErrorMessage::shared_ptr msg(new Commands::ErrorMessage(unhandledRsp));
                    m_errorMessageQueue->enqueue(msg);
                }

                continue;
            }

            if(Commands::Transaction::isLogMessage(unhandledRsp.get()))
            {
                MutexLock lock(m_messageQueueMutex);

                if(m_logMessageQueue)
                {
                    Commands::LogMessage::shared_ptr msg(new Commands::LogMessage(unhandledRsp));
                    m_logMessageQueue->enqueue(msg);
                }

                continue;
            }

            //TRACE("Process Unhandled Rsp %s\n", unhandledRsp->GetFullPktStr());
            bool rspHandled = false;

            for(std::deque<TransactionContainer::shared_ptr>::iterator i = pendingTransactions.begin();
                i != pendingTransactions.end(); ++i)
            {
                Commands::Transaction* transaction = (*i)->m_transaction.get();

                if(transaction->validateRsp(unhandledRsp.get()))
                {
                    TransactionContainer::shared_ptr container = *i;

                    if(unhandledRsp->GetStatus() == COMM_STATUS_INFO)
                    {
                        //TRACE("  Info Rsp %s\n", unhandledRsp->GetFullPktStr());
                        if(container->m_infoRspQueue)
                        {
                            Commands::InfoResponse::shared_ptr infoResponse(new
                                Commands::InfoResponse(container->m_transaction, unhandledRsp));
                            container->m_infoRspQueue->enqueue(infoResponse);
                        }
                    }
                    else
                    {
                        //TRACE("  Completed Rsp %s\n", unhandledRsp->GetFullPktStr());
                        Commands::Transaction::shared_ptr transaction = container->m_transaction;
                        container->m_rspQueue->enqueue(transaction);
                        pendingTransactions.erase(i);
                    }

                    rspHandled = true;
                    break;
                }
            }

            if(!rspHandled)
            {
                TRACE("  Rsp not handled: %s\n", unhandledRsp->GetFullPktStr());
            }
        }
        else
        {
            for(std::deque<TransactionContainer::shared_ptr>::iterator i = pendingTransactions.begin();
                i != pendingTransactions.end(); ++i)
            {
                if((*i)->GetWaitingTime() > TransactionContainer::MAX_WAIT_TIME_ms)
                {
                    TransactionThreadQueue::shared_ptr commRspQueue(new TransactionThreadQueue);

                    Commands::Transaction::shared_ptr cmd(new Commands::GetStatus());
                    sendCommand(commRspQueue, cmd);
                    Commands::Transaction::shared_ptr rsp = commRspQueue->dequeue();

                    if(rsp->validRsp() && rsp->statusOk())
                    {
                        Commands::GetStatus* gs = dynamic_cast<Commands::GetStatus*>(rsp.get());

                        if(gs && gs->pendingProcessCount() == 0)
                        {
                            //Missed OK response
                            TransactionThreadQueue::shared_ptr rspQueue    = (*i)->m_rspQueue;
                            Commands::Transaction::shared_ptr  transaction = (*i)->m_transaction;
                            transaction->setResonse(COMM_STATUS_OK, 0, "");
                            rspQueue->enqueue(transaction);

                            pendingTransactions.erase(i);

                            break;
                        }
                    }

                    (*i)->ResetWaitTime();
                }
            }
        }
    }

    m_pendingThreadState = Thread::State::finished;

    TRACE("Pending Thread exiting\n");
    return 0;
}



void Comm::killCommThreads()
{
    BOOL commThreadStopping     = m_commThreadState.stop();
    BOOL pendingThreadStopping  = m_pendingThreadState.stop();
    BOOL loopbackThreadStopping = m_loopbackThreadState.stop();

    if(commThreadStopping)
    {
        if(WaitForSingleObject(m_commThreadHandle, WAIT_FOR_THREAD_EXIT_TIMEOUT_ms) != WAIT_OBJECT_0)
        {
            //Thread did not die in time - kill it
            TerminateThread(m_commThreadHandle, -1);

            TRACE("Killed COMM thread\n");
        }
    }
 
    if(pendingThreadStopping)
    {
        if(WaitForSingleObject(m_pendingThreadHandle, WAIT_FOR_THREAD_EXIT_TIMEOUT_ms) != WAIT_OBJECT_0)
        {
            //Thread did not die in time - kill it
            TerminateThread(m_pendingThreadHandle, -1);

            TRACE("Killed pending thread\n");
        }
    }

    if(loopbackThreadStopping)
    {
        if(WaitForSingleObject(m_loopbackThreadHandle, WAIT_FOR_THREAD_EXIT_TIMEOUT_ms) != WAIT_OBJECT_0)
        {
            //Thread did not die in time - kill it
            TerminateThread(m_loopbackThreadHandle, -1);

            TRACE("Killed loopback thread\n");
        }
    }
}



void Comm::OnEvent(EEvent events, EError error)
{
    const unsigned BUFFER_SIZE = 2500;

    if(events & EEventError)
    {
        char  buffer[BUFFER_SIZE];
        DWORD readCount;
        LONG  status;

        switch(error)
        {
            case EErrorOverrun: // Character buffer overrun, next byte is lost
                TRACE("Serial RX Error: Character buffer overrun (device)\n");
                // Clear out buffers

                m_rxBuffer.Empty();

                while((status = Read(buffer, BUFFER_SIZE - 1, &readCount)) == ERROR_SUCCESS && readCount > 0)
                {
                }

                break;

            case EErrorRxOver:  // Input buffer overflow, byte lost
                TRACE("Serial RX Error: RX buffer overrun (Windows)\n");
                // Clear out buffers

                m_rxBuffer.Empty();

                while((status = Read(buffer, BUFFER_SIZE - 1, &readCount)) == ERROR_SUCCESS && readCount > 0)
                {
                }

                break;

            case EErrorUnknown: // Unknown
                // Ignore
                break;

            case EErrorBreak:   // Break condition detected
            case EErrorFrame:   // Framing error
            case EErrorIOE:     // I/O device error
            case EErrorMode:    // Unsupported mode
            case EErrorParity:  // Input parity error
            case EErrorTxFull:  // Output buffer full
            default:
                TRACE("Unhandled Serial error: Event = 0x%02x, Error = %d\n", events, error);
                break;
        }
    }
    if(events & EEventRecv)
    {
        char  buffer[BUFFER_SIZE];
        DWORD readCount;
        LONG  status;

        if((status = Read(buffer, BUFFER_SIZE - 1, &readCount)) == ERROR_SUCCESS && readCount > 0)
        {
            ITI_ASSERT_LESS(readCount, BUFFER_SIZE);

            buffer[readCount] = 0;

            char* data = strchr(buffer, CommPkt::START_OF_FRAME);

            if(m_rxBuffer.GetLength())
            {
                if(data)
                {
                    char* eop = strchr(buffer, CommPkt::END_OF_FRAME);
                    if(eop && eop < data)
                    {
                        // pkt end and new pkt starting
                        data = buffer;
                    }
                    else
                    {
                        // Incomplete pkt in buffer with new one starting
                        // Throw out old pkt
                        m_rxBuffer.Empty();
                    }
                }
                else
                {
                    data = buffer;
                }
            }

            if(data)
            {
                m_rxBuffer += data;

                int end;
                while((end = m_rxBuffer.Find(CommPkt::END_OF_FRAME)) > 0)
                {
                    ProcessRxPkt(m_rxBuffer.Left(end + 1));

                    int start = m_rxBuffer.Find(CommPkt::START_OF_FRAME, end);

                    if(start > 0)
                    {
                        m_rxBuffer.Delete(0, start);
                    }
                    else
                    {
                        m_rxBuffer.Empty();
                    }
                }
            }
        }

        if(status != ERROR_SUCCESS)
        {
            TRACE("Serial read error %d\n", status);
        }
    }
    if(events & ~(EEventRecv | EEventError))
    {
        TRACE("Unhandled Serial Event = 0x%02x, Error = %d\n", events, error);
    }
}



void Comm::ProcessRxPkt(const CString& rxStr)
{
    try
    {
        CommPkt::shared_ptr pkt(new CommPkt(rxStr));
        m_rxQueue.enqueue(pkt);
    }
    catch(CommPkt::ExceptionInvalidPkt& ex)
    {
        TRACE("Invalid Rx Pkt: %s '%s'\n", ex.what(), rxStr);
    }
    catch(CommPkt::Exception& ex)
    {
        TRACE("Rx Pkt Error: %s\n", ex.what());
    }
}



DWORD WINAPI Comm::startLoopbackThread(void* commObject)
{
    ITI::ProcessorException::install();

    std::stringstream errMsg;

    try
    {
        return ((Comm*)commObject)->loopbackThread();
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error in Comm Loopback thread:" << std::endl
               << ex.what() << std::endl << "Comm Loopback Thread exited";
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception in Comm Pending thread:" << std::endl
               << ex.what() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Comm Loopback Thread exited";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception in Comm Pending thread:" << std::endl
               << ex.what() << std::endl << "Comm Loopback Thread exited";
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception in Comm Pending thread"
               << std::endl << "Comm Loopback Thread exited";
    }

    ::MessageBox(NULL, errMsg.str().c_str(), "Comm Loopback Thread Error", MB_ICONEXCLAMATION | MB_OK);

    return -1;
}



double randValue()
{
    return (rand() - RAND_MAX / 2) / 1000.0;
}

DWORD Comm::loopbackThread()
{
    m_loopbackThreadState = Thread::State::running;

    TRACE("Loopback Thread running\n");

    while(m_loopbackThreadState.isRunning())
    {
        CommPkt::shared_ptr cmd = m_loopbackTxQueue.dequeue(1);
        if(!cmd)
        {
            continue;
        }

        TRACE("TX Pkt: %s\n", cmd->GetFullPktStr());

        CommPkt::shared_ptr rsp(new CommPkt());

        rsp->AddData(HEADER_TO_HOST_FROM_MASTER);
        rsp->AddData(cmd->GetSeqNum());
        rsp->AddData(cmd->GetCmd());


        if(cmd->GetCmd() == Commands::GET_FW_VERSION_STR)
        {
            rsp->AddData(COMM_STATUS_OK);
            rsp->AddData("Loopback");
            rsp->AddData(__DATE__);
            rsp->AddData(__TIME__);
        }
        else if(cmd->GetCmd() == Commands::GET_CURRENT_TEMP_STR)
        {
            rsp->AddData(COMM_STATUS_OK);
            rsp->AddData(cmd->GetData(0));
            if(cmd->GetData(0) == Commands::CHANNEL_BLOCK)
            {
                for(int i = 0; i < CHANNEL_COUNT; ++i)
                {
                    rsp->AddData("%.3f", randValue());
                }
            }
            else
            {
                rsp->AddData("%.3f", randValue());
            }
        }
        else if(cmd->GetCmd() == Commands::SET_TARGET_TEMP_STR)
        {
            rsp->AddData(COMM_STATUS_OK);
        }
        else
        {
            rsp->AddData(COMM_STATUS_ERROR);
            rsp->AddData("Unknown Command");
        }

        m_rxQueue.enqueue(rsp);
    }

    m_loopbackThreadState = Thread::State::finished;

    TRACE("Loopback Thread exiting\n");
    return 0;
}






// EOF
