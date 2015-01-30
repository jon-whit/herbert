#ifndef InstrumentEvent_h
#define InstrumentEvent_h


#include <boost/shared_ptr.hpp>
#include <Exceptions.h>
#include <ThreadQueue.h>



class InstrumentEvent
{
public:
    enum Type
    {
        Type_ErrorMsg,
        Type_LogMsg,
        Type_InstrumentException,
        Type_FPGAInfoUpdate,
        Type_PCBInfoUpdate,
    };
    
    typedef boost::shared_ptr<InstrumentEvent> shared_ptr;

    InstrumentEvent()  {}
    ~InstrumentEvent() {}

    virtual Type type() = 0;
};



typedef ThreadQueue<InstrumentEvent::shared_ptr>                 InstrumentEventQueue;
typedef GuiMessagingThreadQueue<InstrumentEvent::shared_ptr>     GuiInstrumentEventQueue;


//-------------------------------------------------------
class ErrorMessage : public InstrumentEvent
{
public:
    ErrorMessage(int number, const std::string& msg) :
        m_number(number), m_msg(msg)
        {}
    ~ErrorMessage() {}
    virtual Type type() {return Type_ErrorMsg;}

    int                number() {return m_number;}
    const std::string& msg()    {return m_msg;}

private:
    int         m_number;
    std::string m_msg;
};

//-------------------------------------------------------
class LogMessage : public InstrumentEvent
{
public:
    LogMessage(const std::string& msg) :
        m_msg(msg)
        {}
    ~LogMessage() {}
    virtual Type type() {return Type_LogMsg;}

    const std::string& msg()    {return m_msg;}

private:
    std::string m_msg;
};

//-------------------------------------------------------
class InstrumentException : public InstrumentEvent
{
public:
    InstrumentException(const std::string& msg) :
        m_msg(msg)
        {}
    ~InstrumentException() {}
    virtual Type type() {return Type_InstrumentException;}

    const std::string& msg()   {return m_msg;}

private:
    std::string m_msg;
};


//-------------------------------------------------------
class FPGAInfoUpdate : public InstrumentEvent
{
public:
    FPGAInfoUpdate(const std::string& firmwareVersion,
                   const std::string& firmwareBuildDate,
                   const std::string& fpgaVersion) :
        m_firmwareVersion(firmwareVersion),
        m_firmwareBuildDate(firmwareBuildDate),
        m_fpgaVersion(fpgaVersion)
        {}
    ~FPGAInfoUpdate() {}
    virtual Type type() {return Type_FPGAInfoUpdate;}

    const std::string& firmwareVersion()   {return m_firmwareVersion;}
    const std::string& firmwareBuildDate() {return m_firmwareBuildDate;}
    const std::string& fpgaVersion()       {return m_fpgaVersion;}

private:
    std::string m_firmwareVersion;
    std::string m_firmwareBuildDate;
    std::string m_fpgaVersion;
};

//-------------------------------------------------------
class PCBInfoUpdate : public InstrumentEvent
{
public:
    PCBInfoUpdate(const std::string& firmwareVersion,
                  const std::string& firmwareBuildDate) :
        m_firmwareVersion(firmwareVersion),
        m_firmwareBuildDate(firmwareBuildDate)
        {}
    ~PCBInfoUpdate() {}
    virtual Type type() {return Type_PCBInfoUpdate;}

    const std::string& firmwareVersion()   {return m_firmwareVersion;}
    const std::string& firmwareBuildDate() {return m_firmwareBuildDate;}

private:
    std::string m_firmwareVersion;
    std::string m_firmwareBuildDate;
};



#endif