#ifndef Instrument_h
#define Instrument_h


#include <boost/shared_ptr.hpp>
#include <InstrumentEvent.h>
#include <FpgaComm.h>
#include <PcbComm.h>
#include <Exceptions.h>
#include <ThreadQueue.h>
#include <enumser.h>
#include <vector>



class Instrument
{
public:
    class Exception : public ITI::Exception
    {
    public:
        inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };

    class ConnectionException : public std::runtime_error
    {
    public:
        inline ConnectionException(const std::string& ex) : std::runtime_error(ex) {}
    };



public:
    static Instrument& getInstance();
    ~Instrument();

    void connect(const std::string& fpgaCommPort, const std::string& pcbCommPort);
    void disconnect();
    bool connected()     {return m_fpgaComm.IsOpen() || m_pcbComm.IsOpen();}
    bool fpgaConnected() {return m_fpgaComm.IsOpen();}
    bool pcbConnected()  {return m_pcbComm.IsOpen();}

    void updateSerialPorts();
    void getSerialPorts(std::vector<std::string>* ports);


    static void updateFPGAInfo();
    void        watchFPGAInfoUpdate(InstrumentEventQueue::shared_ptr queue);

    static void updatePCBInfo();
    void        watchPCBInfoUpdate(InstrumentEventQueue::shared_ptr queue);

    void        watchErrorMessages(InstrumentEventQueue::shared_ptr queue);
    void        watchLogMessages(InstrumentEventQueue::shared_ptr queue);
    void        watchExceptions(InstrumentEventQueue::shared_ptr queue);

    std::string getHDImageVersion();
    std::string getSystemSerialNumber();


private:
    enum Constants
    {
        WAIT_FOR_THREAD_EXIT_TIMEOUT_ms = 500,
        DEVICE_INFO_POLL_PERIOD_ms      = 2000
    };

    Instrument();
    Instrument(Instrument&);

    Mutex                             m_mutex;
    FpgaComm&                         m_fpgaComm;
    PcbComm&                          m_pcbComm;
    InstrumentEventQueue::shared_ptr  m_exceptionQueue;

    std::vector<std::string>          m_serialPorts;


    ///////////////////////////////////////////////////////////////////////////////////
    // Worker Threads

    void killInstrumentThreads();


    friend class InstrumentThread;
    class InstrumentThread : public Thread
    {
    public:
        InstrumentThread(const std::string& name, Instrument& instrument, Comm& comm) :
          Thread(name), m_instrument(instrument), m_comm(comm)
        {}

        bool startThread();
        void setEventQueue(InstrumentEventQueue::shared_ptr queue);

    protected:
        void                              start() {Thread::start();}
        void                              handleException(const std::string& msg);
        Commands::Transaction::shared_ptr executeCmd(Commands::Transaction::shared_ptr cmd);

        Mutex                             m_mutex;
        InstrumentEventQueue::shared_ptr  m_eventQueue;
        Instrument&                       m_instrument;
        Comm&                             m_comm;
    };



    ///////////////////////////////////////////////////////////////////////////////////
    // Error Message Thread
    class ErrorMessageThread : public InstrumentThread
    {
    public:
        ErrorMessageThread(Instrument& instrument, Comm& comm) :
          InstrumentThread("ErrorMessageThread", instrument, comm) {}

    private:
        DWORD thread();
    };
        
    ErrorMessageThread m_errorMessageThread;



    ///////////////////////////////////////////////////////////////////////////////////
    // Log Message Thread
    class LogMessageThread : public InstrumentThread
    {
    public:
        LogMessageThread(Instrument& instrument, Comm& comm) :
          InstrumentThread("LogMessageThread", instrument, comm) {}

    private:
        DWORD thread();
     };
        
    LogMessageThread m_logMessageThread;



    ///////////////////////////////////////////////////////////////////////////////////
    // FPGA Info
    class FPGAInfoThread : public InstrumentThread
    {
    public:
        FPGAInfoThread(Instrument& instrument, Comm& comm) :
          InstrumentThread("FPGAInfoThread", instrument, comm) {}

    private:
        DWORD thread();
        virtual bool  exitSleep()  { return !m_comm.IsOpen() || !isRunning(); }
        virtual bool  exitThread() { return !m_comm.IsOpen() || !isRunning(); }

        void sendInfo(const std::string& firmwareVersion,
                      const std::string& firmwareBuildDate,
                      const std::string& fpgaVersion);
    };

    FPGAInfoThread m_fpgaInfoThread;



    ///////////////////////////////////////////////////////////////////////////////////
    // PCB Info
    class PCBInfoThread : public InstrumentThread
    {
    public:
        PCBInfoThread(Instrument& instrument, Comm& comm) :
          InstrumentThread("PCBInfoThread", instrument, comm) {}

    private:
        DWORD thread();
        virtual bool  exitSleep()  { return !m_comm.IsOpen() || !isRunning(); }
        virtual bool  exitThread() { return !m_comm.IsOpen() || !isRunning(); }

        void sendInfo(const std::string& firmwareVersion,
                      const std::string& firmwareBuildDate);
    };
        
    PCBInfoThread m_pcbInfoThread;
};



#endif