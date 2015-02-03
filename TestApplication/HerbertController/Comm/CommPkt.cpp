/////////////////////////////////////////////////////////
//
//  CommPkt.cpp
//
//  Serial Communication Pkt class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include "CommPkt.h"
#include <Commands.h>

unsigned CommPkt::m_sequenceNumberSeed = 0;



CommPkt::CommPkt()
{
}



CommPkt::CommPkt(const CString& fullPkt)
    : m_originalPkt(fullPkt)
{
    AddData(ValidatePkt(fullPkt));
}



void CommPkt::ShowPkt()
{
    TRACE("Header:  %s\n", m_header);
    TRACE("Command: %s\n", m_cmd);
    #if USE_SEQ_NUMBER
        TRACE("Seq Num: %s\n", m_seqNum);
    #endif
    int i;
    for(i = 0; i < m_data.GetSize() - 1; i++)
    {
        TRACE("Data:    %s\n", m_data[i]);
    }
    TRACE("CRC:     %s\n", m_data[i]);
}



CString CommPkt::GetFullPktStr()
{
    CString pkt;

    if(m_header.IsEmpty())
    {
        m_header = HEADER_TO_MASTER_FROM_HOST;
    }

    #if USE_SEQ_NUMBER
        if(m_seqNum.IsEmpty())
        {
            throw Exception("Error: Missing sequence number");
        }
    #endif

    if(m_cmd.IsEmpty())
    {
        throw Exception("Error: Missing command");
    }

    pkt = m_header;
    
    #if USE_SEQ_NUMBER
        pkt += ' ';
        pkt += m_seqNum;
    #endif
    
    pkt += ' ';
    pkt += m_cmd;

    for(int i = 0; i < m_data.GetSize(); i++)
    {
        pkt += ' ';
        pkt += m_data[i];
    }

    CRC16 crc = calcCRC16(INITIAL_CRC16_VALUE, pkt, pkt.GetLength());

    CString crcStr;
    crcStr.Format("%d", crc);

    pkt += ' ';
    pkt += crcStr;


    pkt.Insert(0, START_OF_FRAME);
    pkt += (char)END_OF_FRAME;

    return pkt;
}



CString CommPkt::GetData(int index) const
{
    if(index < 0 || index >= m_data.GetSize())
    {
        throw Exception("Index out of range");
    }

    return m_data[index];
}



CString CommPkt::GetStatus() const
{
    if(m_data.GetSize() > COMM_STATUS_INDEX)
    {
        return GetData(COMM_STATUS_INDEX);
    }
    else
    {
        return CString("");
    }
}


int CommPkt::GetErrorCode() const
{
    if(m_data.GetSize() > COMM_ERROR_CODE_INDEX &&
       ((GetStatus() == COMM_STATUS_ERROR) ||
        (GetStatus() == COMM_STATUS_INFO && GetCmd() == Commands::ERROR_MSG_KEY_STR)))
    {
        char* endPtr;
        int errorCode = strtol(GetData(COMM_ERROR_CODE_INDEX), &endPtr, 10);

        if(!*endPtr)
            return errorCode;
    }

    return 0;
}


CString CommPkt::GetErrorString() const
{
    CString errorString;
    if(m_data.GetSize() > COMM_ERROR_STR_INDEX &&
        ((GetStatus() == COMM_STATUS_ERROR) ||
        (GetStatus() == COMM_STATUS_INFO && GetCmd() == Commands::ERROR_MSG_KEY_STR)))
   {
        for(int i = COMM_ERROR_STR_INDEX; i < m_data.GetSize(); i++)
        {
            if(!errorString.IsEmpty())
            {
                errorString += ' ';
            }
            errorString += m_data[i];
        }
    }

    return errorString;
}



void CommPkt::Clear()
{
    m_header.Empty();
    m_cmd.Empty();
    m_data.RemoveAll();
}



CommPkt& CommPkt::AddData(const char* format, ...)
{
    CString data;
    va_list argList;

    va_start(argList, format);
    data.FormatV(format, argList);
    va_end(argList);

    AddData(data);

    return *this;
}



CString CommPkt::ValidatePkt(const CString& fullPkt)
{
    CString pkt = fullPkt;

    // Check for proper pkt frame
    if(pkt[0] != START_OF_FRAME)
    {
        throw ExceptionInvalidPkt("Missing Start of Frame");
    }

    if(pkt.Right(1) != END_OF_FRAME)
    {
        throw ExceptionInvalidPkt("Missing End of Frame");
    }

    pkt.Delete(0);
    pkt.Delete(pkt.GetLength() - 1);

    if(pkt.Find(START_OF_FRAME) >= 0 || pkt.Find(END_OF_FRAME) >= 0)
    {
        throw ExceptionInvalidPkt("Malformed packet");
    }


    // Check CRC
    int crcIndex = pkt.ReverseFind(' ');

    if(crcIndex < 0)
    {
        throw ExceptionInvalidPkt("Missing CRC");
    }

    CString crcStr = pkt.Mid(crcIndex + 1);

    pkt = pkt.Left(crcIndex);

    pkt.TrimLeft();
    pkt.TrimRight();

    if(pkt.IsEmpty())
    {
        throw ExceptionInvalidPkt("Missing Pkt data");
    }

    CRC16 calcCrc = calcCRC16(INITIAL_CRC16_VALUE, pkt, pkt.GetLength());
    CRC16 pktCrc  = atoi(crcStr);
    if(pktCrc != 0 && pktCrc != calcCrc)
    {
        CString msg;
        msg.Format("Invalid CRC: %s\n", fullPkt);
        TRACE(msg);
        throw ExceptionInvalidPkt((LPCSTR)msg);
    }

#if USE_SEQ_NUMBER
    int seqNumIndex;

    // Check for command
    if((seqNumIndex = pkt.Find(' ')) < 0)
    {
        throw ExceptionInvalidPkt("Missing Sequence Number");
    }

    // Check for command
    if(pkt.GetLength() > seqNumIndex + 1 && pkt.Find(' ', seqNumIndex + 1) < 0)
    {
        throw ExceptionInvalidPkt("Missing Command");
    }
#else
    // Check for command
    if(pkt.Find(' ') < 0)
    {
        throw ExceptionInvalidPkt("Missing Command");
    }
#endif

    return pkt;
}


CommPkt& CommPkt::AddData(const CString& data)
{
    CString pkt = data;

    int itemIndexStart = 0;
    int itemIndexEnd   = 0;

    while(itemIndexStart >= 0 &&
          itemIndexStart < pkt.GetLength() &&
          itemIndexEnd >= 0)
    {
        if(pkt[itemIndexStart] == '"')
        {
            itemIndexEnd = pkt.Find('"', itemIndexStart + 1);

            if(itemIndexEnd > 0)
            {
                itemIndexEnd++;
            }
        }
        else
        {
            itemIndexEnd = pkt.Find(' ', itemIndexStart);
        }

        CString item = itemIndexEnd > 0 ?
                       pkt.Mid(itemIndexStart, itemIndexEnd - itemIndexStart) :
                       pkt.Mid(itemIndexStart);

        item.TrimLeft();
        item.TrimRight();

        if(m_header.IsEmpty())
        {
            m_header = item;
        }

        #if USE_SEQ_NUMBER
            else if(m_seqNum.IsEmpty())
            {
                m_seqNum = item;
            }
        #endif

        else if(m_cmd.IsEmpty())
        {
            m_cmd = item;
        }
        else
        {
            m_data.Add(item);
        }

        itemIndexStart = itemIndexEnd + 1;
    }

    return *this;
}
