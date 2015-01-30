/////////////////////////////////////////////////////////////
//
//  Timestamp.cpp
//
//  Timestamp class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#include <Timestamp.h>
#include <StdAfx.h>


Timestamp::Timestamp()
{
    time_t tsTime;
    time(&tsTime);

    MutexLock lock(m_mutex);
    struct tm* utcTime = gmtime(&tsTime);

    m_year   = utcTime->tm_year + 1900; // 1900 as per definition of struct tm
    m_month  = utcTime->tm_mon + 1;     // Make 1 based
    m_day    = utcTime->tm_mday;
    m_hour   = utcTime->tm_hour;
    m_minute = utcTime->tm_min;
    m_second = utcTime->tm_sec;

}

Timestamp::Timestamp(uint16 year,
                     uint8  month,
                     uint8  day,
                     uint8  hour,
                     uint8  minute,
                     uint8  second)
{
    m_year = year;
    m_month = month;
    m_day = day;
    m_hour = hour;
    m_minute = minute;
    m_second = second;
}

