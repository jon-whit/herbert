/////////////////////////////////////////////////////////////
//
//  Timestamp.h
//
//  Timestamp class
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef Timestamp_h
#define Timestamp_h


#include <time.h>
#include <types.h>
#include <Mutex.h>


class Timestamp
{
public:
    Timestamp();
    Timestamp(uint16 year,
              uint8  month,
              uint8  day,
              uint8  hour,
              uint8  minute,
              uint8  second);

    ~Timestamp() {}

    uint16 m_year;
    uint8  m_month;
    uint8  m_day;
    uint8  m_hour;
    uint8  m_minute;
    uint8  m_second;

    Mutex  m_mutex;
};

#endif