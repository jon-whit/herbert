////////////////////////////////////////////////////////////////
//
//  ThreadData.h
//
//  Thread-safe data object
//
//  Copyright 2009 Brett C. Gilbert

#ifndef ThreadData_h
#define ThreadData_h

#include <Mutex.h>
#include <Semaphore.h>



template <class T>
class ThreadData
{
public:
    //Exceptions

public:
    ThreadData();
    ThreadData(const T& value);
    ThreadData(ThreadData<T>& data);
    inline T operator=(const T& value);
    inline T operator=(ThreadData<T>& data);

    ~ThreadData() {}

    inline operator T();

    inline T getValue();
    inline T setValue(T value);

private:
    T     m_data;
    Mutex m_mutex;
};



template <class T>
ThreadData<T>::ThreadData()
{
}



template <class T>
ThreadData<T>::ThreadData(const T& value)
    : m_data(value)
{
}



template <class T>
ThreadData<T>::ThreadData(ThreadData<T>& data)
    : m_data(data.getValue())
{
}



template <class T>
T ThreadData<T>::operator=(const T& value)
{
    return setValue(value);
}



template <class T>
T ThreadData<T>::operator=(ThreadData<T>& data)
{
    return setValue(data.getValue());
}



template <class T>
ThreadData<T>::operator T()
{
    return getValue();
}



template <class T>
T ThreadData<T>::getValue()
{
    MutexLock lock(m_mutex);
    return m_data;
}



template <class T>
T ThreadData<T>::setValue(T value)
{
    MutexLock lock(m_mutex);
    m_data = value;
    return m_data;
}


#endif
