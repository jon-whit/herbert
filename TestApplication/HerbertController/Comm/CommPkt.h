/////////////////////////////////////////////////////////
//
//  CommPkt.h
//
//  Serial Communication Pkt class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef CommPkt_h
#define CommPkt_h

#include <stdexcept>
#include <string>
#include <afxtempl.h>
#include <crc16.h>
#include <boost/shared_ptr.hpp>
#include <Exceptions.h>


#define USE_SEQ_NUMBER 1


#define HEADER_TO_HOST_FROM_MASTER "HM"
#define HEADER_TO_MASTER_FROM_HOST "MH"

#define HEADER_TO_SBC_FROM_PRB     "SP"
#define HEADER_TO_PRB_FROM_SBC     "PS"

#define COMM_CMD_FILE_SEND     "FileSend"
#define COMM_CMD_UPGRADE       "Upgrade"

#define COMM_STATUS_OK         "OK"
#define COMM_STATUS_PENDING    "PE"
#define COMM_STATUS_ERROR      "ER"
#define COMM_STATUS_INFO       "IP"

#define MAX_FILE_DATA_SIZE     900

#define CHANNEL_COUNT          24
#define WELL_PER_CHANNEL       4

#define LID_CHANNEL_COUNT      2


#define COMM_STATUS_INDEX      0
#define COMM_ERROR_CODE_INDEX  1
#define COMM_ERROR_STR_INDEX   2



class CommPkt
{
public:
    class Exception : public ITI::Exception
    {
        public:
            inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };

    class ExceptionInvalidPkt : public Exception
    {
        public:
            inline ExceptionInvalidPkt(const std::string& ex) : Exception(ex) {}
    };

    
    enum Frame
    {
        START_OF_FRAME = '[',
        END_OF_FRAME   = ']'
    };

    enum Header
    {
        HEADER_HOST_TO_MASTER 
    };

    typedef boost::shared_ptr<CommPkt> shared_ptr;



    CommPkt();
    CommPkt(const CString& fullPkt);
    ~CommPkt() {}

    CString GetFullPktStr();
    CString GetOriginalPktStr() { if(m_originalPkt.IsEmpty()){m_originalPkt = GetFullPktStr();} return m_originalPkt; }
    CommPkt& CreateFromString(const CString& pkt);

    void Clear();

    CommPkt& SetHeader(const CString& header) { m_header = header;     return *this; }
    CommPkt& InitSeqNum()                     { m_sequenceNumberSeed = (m_sequenceNumberSeed + 1) % 0x100;
                                                m_seqNum.Format("%02X", m_sequenceNumberSeed);
                                                return *this; }
    CommPkt& SetCmd(const CString& cmd)       { m_cmd = cmd;           return *this; }
    CommPkt& AddData(const CString& data);
    CommPkt& AddData(const char* format, ...);

    inline int GetDataLen() { return m_data.GetSize(); }

    CString GetHeader() const { return m_header; }
    CString GetCmd() const { return m_cmd; }
    CString GetSeqNum() const { return m_seqNum; }
    CString GetData(int index) const;

    CString GetStatus() const;
    int     GetErrorCode() const;
    CString GetErrorString() const;

    CString operator[](int index) const { return GetData(index); }


    //Debug commands
    void ShowPkt();

private:
    CString ValidatePkt(const CString& fullPkt);

    CString      m_originalPkt;    
    CString      m_header;
    CString      m_cmd;
    CString      m_seqNum;
    CStringArray m_data;

    static unsigned m_sequenceNumberSeed;
};


#endif

