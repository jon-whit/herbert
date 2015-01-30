#include <Instrument.h>
#include <Logging.h>
#include <StringFormatting.h>

const static char NO_SERIAL_PORT_STR[] = "None";



Instrument::Instrument() :
    m_fpgaComm(FpgaComm::getInstance()),
    m_pcbComm(PcbComm::getInstance()),
    m_errorMessageThread(*this, m_fpgaComm),
    m_logMessageThread(*this, m_fpgaComm),
    m_fpgaInfoThread(*this, m_fpgaComm),
    m_pcbInfoThread(*this, m_pcbComm)
{
    m_errorMessageThread.startThread();
    m_logMessageThread.startThread();

    updateSerialPorts();
}

Instrument::~Instrument()
{
    killInstrumentThreads();
}

Instrument& Instrument::getInstance()
{
    static Instrument instance;
    return instance;
}


void Instrument::connect(const std::string& fpgaCommPort, const std::string& pcbCommPort)
{
    if(fpgaCommPort == NO_SERIAL_PORT_STR && pcbCommPort == NO_SERIAL_PORT_STR)
    {
        throw ConnectionException("No Port Specified");
    }

    if(fpgaCommPort == pcbCommPort)
    {
        throw ConnectionException("FPGA & PCB Ports cannot be the same");
    }

    if(fpgaCommPort != NO_SERIAL_PORT_STR)
    {
        m_fpgaComm.Open(fpgaCommPort);

        if(!m_fpgaComm.IsOpen())
        {
            std::stringstream errorMsg;
            errorMsg << "Unable to open FPGA COMM port (" << fpgaCommPort << ")";
            throw ConnectionException(errorMsg.str());
        }
    }

    if(pcbCommPort != NO_SERIAL_PORT_STR)
    {
        m_pcbComm.Open(pcbCommPort);

        if(!m_pcbComm.IsOpen())
        {
            if(m_fpgaComm.IsOpen())
            {
                Sleep(100);
                m_fpgaComm.Close();
            }

            std::stringstream errorMsg;
            errorMsg << "Unable to open PCB COMM port (" << pcbCommPort << ")";
            throw ConnectionException(errorMsg.str());
        }
    }

    m_fpgaInfoThread.startThread();
    m_pcbInfoThread.startThread();
}


void Instrument::disconnect()
{
    if(m_fpgaComm.IsOpen()) m_fpgaComm.Close();
    if(m_pcbComm.IsOpen())   m_pcbComm.Close();
}


void Instrument::updateSerialPorts()
{
    CEnumerateSerial::UsingRegistry(m_serialPorts);

    std::sort(m_serialPorts.begin(), m_serialPorts.end(), naturalStringCompare);

    m_serialPorts.insert(m_serialPorts.begin(), NO_SERIAL_PORT_STR);
}

void Instrument::getSerialPorts(std::vector<std::string>* ports)
{
    updateSerialPorts();
    *ports = m_serialPorts;
}


void Instrument::watchExceptions(InstrumentEventQueue::shared_ptr queue)
{
    MutexLock lock(m_mutex);
    m_exceptionQueue = queue;
}

//---------------------------------------------------------------------
//  Threads
//
void Instrument::killInstrumentThreads()
{
    m_errorMessageThread.stop();
    m_logMessageThread.stop();
    m_fpgaInfoThread.stop();
    m_pcbInfoThread.stop();

    m_errorMessageThread.waitForStop();
    m_logMessageThread.waitForStop();
    m_fpgaInfoThread.waitForStop();
    m_pcbInfoThread.waitForStop();
}


void Instrument::InstrumentThread::handleException(const std::string& msg)
{
    log(msg);
    {
        MutexLock lock(m_instrument.m_mutex);
        if(m_instrument.m_exceptionQueue)
        {
            InstrumentException::shared_ptr ie(new InstrumentException(msg));
            m_instrument.m_exceptionQueue->enqueue(ie);
            return;
        }
    }
}

void Instrument::InstrumentThread::setEventQueue(InstrumentEventQueue::shared_ptr queue)
{
    MutexLock lock(m_mutex);
    m_eventQueue = queue;
}

bool Instrument::InstrumentThread::startThread()
{
    MutexLock lock(m_mutex);
    if(!isActive())
    {
        Thread::start();
        return true;
    }
    return false;
}

Commands::Transaction::shared_ptr Instrument::InstrumentThread::executeCmd(Commands::Transaction::shared_ptr cmd)
{
    TransactionThreadQueue::shared_ptr rspQueue(new TransactionThreadQueue());
    m_comm.sendCommand(rspQueue, cmd);

    Commands::Transaction::shared_ptr transaction;
    while(1)
    {
        if(transaction || exitThread())
        {
            break;
        }

        transaction = rspQueue->dequeue(1);
    }

    return transaction;
}




//---------------------------------------------------------------------
//  Error Message

void Instrument::watchErrorMessages(InstrumentEventQueue::shared_ptr queue)
{
    m_errorMessageThread.setEventQueue(queue);
}

DWORD Instrument::ErrorMessageThread::thread()
{
    ErrorMessageThreadQueue::shared_ptr messageQueue(new ErrorMessageThreadQueue());

    m_comm.setErrorQueue(messageQueue);

    while(!exitThread())
    {
        Commands::ErrorMessage::shared_ptr errorMessage = messageQueue->dequeue(1);

        if(errorMessage)
        {
            log("FPGA Error: %s (%03d)", LPCSTR(errorMessage->string()), errorMessage->number());

            MutexLock lock(m_mutex);
            if(m_eventQueue)
            {
                InstrumentEvent::shared_ptr ie(new ErrorMessage(errorMessage->number(), LPCSTR(errorMessage->string())));
                m_eventQueue->enqueue(ie);
            }
        }
    }

    return 0;
}




//---------------------------------------------------------------------
//  Log Message

void Instrument::watchLogMessages(InstrumentEventQueue::shared_ptr queue)
{
    m_logMessageThread.setEventQueue(queue);
}

DWORD Instrument::LogMessageThread::thread()
{
    LogMessageThreadQueue::shared_ptr messageQueue(new LogMessageThreadQueue());

    m_comm.setLogQueue(messageQueue);

    while(!exitThread())
    {
        Commands::LogMessage::shared_ptr logMessage = messageQueue->dequeue(1);

        if(logMessage)
        {
            log("FPGA Log: %s", LPCSTR(logMessage->string()));

            MutexLock lock(m_mutex);
            if(m_eventQueue)
            {
                InstrumentEvent::shared_ptr ie(new LogMessage(LPCSTR(logMessage->string())));
                m_eventQueue->enqueue(ie);
            }
        }
    }

    return 0;
}


//---------------------------------------------------------------------
//  FPGA Info

void Instrument::updateFPGAInfo()
{
    Instrument::getInstance().m_fpgaInfoThread.startThread();
}

void Instrument::watchFPGAInfoUpdate(InstrumentEventQueue::shared_ptr queue)
{
    m_fpgaInfoThread.setEventQueue(queue);
}

void Instrument::FPGAInfoThread::sendInfo(const std::string& firmwareVersion,
                                          const std::string& firmwareBuildDate,
                                          const std::string& fpgaVersion)
{
    MutexLock lock(m_mutex);
    if(m_eventQueue)
    {
        InstrumentEvent::shared_ptr ie(new FPGAInfoUpdate(firmwareVersion, firmwareBuildDate, fpgaVersion));
        m_eventQueue->enqueue(ie);
    }
}



DWORD Instrument::FPGAInfoThread::thread()
{
    TRACE("FPGA Info Thread running\n");

    sendInfo("...", "...", "...");

    std::string firmwareVersion;
    std::string firmwareBuildDate;
    std::string fpgaVersion;

    while(!exitThread())
    {
        if(firmwareVersion.empty())
        {
            Commands::Transaction::shared_ptr cmd(new Commands::GetFpgaFirmwareVersion());
            Commands::Transaction::shared_ptr transaction = executeCmd(cmd);
            if(transaction)
            {
                if(transaction->type() == Commands::Transaction::TypeGetFpgaFirmwareVersion)
                {
                    Commands::GetFpgaFirmwareVersion* rsp = dynamic_cast<Commands::GetFpgaFirmwareVersion*>(transaction.get());
                    ITI_ASSERT_NOT_NULL(rsp);

                    if(rsp->statusOk())
                    {
                        firmwareVersion   = rsp->firmwareVersion();
                        firmwareBuildDate = rsp->buildDate();
                        continue;
                    }
                }
            }

            threadSleep(Instrument::DEVICE_INFO_POLL_PERIOD_ms);
            continue;
        }

        if(fpgaVersion.empty())
        {
            Commands::Transaction::shared_ptr cmd(new Commands::GetFpgaVersion());
            Commands::Transaction::shared_ptr transaction = executeCmd(cmd);
            if(transaction)
            {
                if(transaction->type() == Commands::Transaction::TypeGetFpgaVersion)
                {
                    Commands::GetFpgaVersion* rsp = dynamic_cast<Commands::GetFpgaVersion*>(transaction.get());
                    ITI_ASSERT_NOT_NULL(rsp);

                    if(rsp->statusOk())
                    {
                        fpgaVersion = rsp->fpgaVersion();
                        continue;
                    }
                }
            }

            threadSleep(Instrument::DEVICE_INFO_POLL_PERIOD_ms);
            continue;
        }

        sendInfo(firmwareVersion, firmwareBuildDate, fpgaVersion);
        break;
    }

    return 0;
}




//---------------------------------------------------------------------
//  PCB Info

void Instrument::updatePCBInfo()
{
    Instrument::getInstance().m_pcbInfoThread.startThread();
}

void Instrument::watchPCBInfoUpdate(InstrumentEventQueue::shared_ptr queue)
{
    m_pcbInfoThread.setEventQueue(queue);
}

void Instrument::PCBInfoThread::sendInfo(const std::string& firmwareVersion,
                                         const std::string& firmwareBuildDate)
{
    MutexLock lock(m_mutex);
    if(m_eventQueue)
    {
        InstrumentEvent::shared_ptr ie(new PCBInfoUpdate(firmwareVersion, firmwareBuildDate));
        m_eventQueue->enqueue(ie);
    }
}

DWORD Instrument::PCBInfoThread::thread()
{
    sendInfo("...", "...");

    std::string firmwareVersion;
    std::string firmwareBuildDate;

    while(!exitThread())
    {
        if(firmwareVersion.empty())
        {
            Commands::Transaction::shared_ptr cmd(new Commands::GetPcbFirmwareVersion());
            Commands::Transaction::shared_ptr transaction = executeCmd(cmd);
            if(transaction)
            {
                if(transaction->type() == Commands::Transaction::TypeGetPcbFirmwareVersion)
                {
                    Commands::GetPcbFirmwareVersion* rsp = dynamic_cast<Commands::GetPcbFirmwareVersion*>(transaction.get());
                    ITI_ASSERT_NOT_NULL(rsp);

                    if(rsp->statusOk())
                    {
                        firmwareVersion   = rsp->firmwareVersion();
                        firmwareBuildDate = rsp->buildDate();
                        continue;
                    }
                }
            }

            threadSleep(Instrument::DEVICE_INFO_POLL_PERIOD_ms);
            continue;
        }

        sendInfo(firmwareVersion, firmwareBuildDate);
        break;
    }

    return 0;
}


std::string Instrument::getHDImageVersion()
{
    HKEY registryKey;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Idaho Technology\\Olympia\\WES",
                    0, KEY_READ, &registryKey ) == ERROR_SUCCESS)
    {
        char imageVersion[128] = "";
        DWORD dataType  = REG_SZ;
        DWORD dataCount = sizeof(imageVersion);

        if(RegQueryValueEx(registryKey, "Image Version", 0, &dataType,
                          (unsigned char*)imageVersion, &dataCount ) == ERROR_SUCCESS)
        {
            return imageVersion;
        }
    }

    return "Unknown";
}


std::string Instrument::getSystemSerialNumber()
{
    HKEY registryKey;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Idaho Technology\\Olympia",
                    0, KEY_READ, &registryKey ) == ERROR_SUCCESS)
    {
        char serialNumber[128] = "";
        DWORD dataType  = REG_SZ;
        DWORD dataCount = sizeof(serialNumber);

        if(RegQueryValueEx(registryKey, "SerialNumber", 0, &dataType,
                          (unsigned char*)serialNumber, &dataCount ) == ERROR_SUCCESS)
        {
            return serialNumber;
        }
    }

    return "Unknown";
}


