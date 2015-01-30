////////////////////////////////////////////////////////////////
//
//  ThreadQueue.h
//
//  Thread-safe queue for inter-thread data passing
//
//  Copyright 2008 Brett C. Gilbert


#ifndef ThreadQueue_h
#define ThreadQueue_h


#pragma warning( disable : 4355 )  // It is okay to use 'this' in member initializer



#include <Mutex.h>
#include <Semaphore.h>
#include <list>
#include <boost/shared_ptr.hpp>


//////////////////////////////////////////////
// Template class definitions

template <class T>
class ThreadQueue
{
public:
    //Exceptions

public:
    ThreadQueue();
    virtual ~ThreadQueue();

    int count();

    virtual T enqueue(T element);
    virtual T dequeue();
    virtual T dequeue(unsigned timeout_ms);

    typedef boost::shared_ptr<ThreadQueue<T>> shared_ptr;

private:
    // Explicitly disallow copy constructor and operator=
    ThreadQueue(const ThreadQueue<T>& );
    ThreadQueue<T>& operator=(const ThreadQueue<T>&);

    std::list<T> m_list;
    Mutex         m_mutex;
    Semaphore     m_semaphore;
};


template <class T>
class GuiMessagingThreadQueue : public ThreadQueue<T>
{
public:
    //Exceptions

public:
    GuiMessagingThreadQueue(CWnd* gui, UINT message);
    ~GuiMessagingThreadQueue() {}

    T enqueue(T element);
    T dequeue(); // Only have Non-blocking dequeue

private:
    // Explicitly disallow default and copy constructors and operator=
    GuiMessagingThreadQueue();
    GuiMessagingThreadQueue(const GuiMessagingThreadQueue<T>&);
    GuiMessagingThreadQueue<T>& operator=(const GuiMessagingThreadQueue<T>&);
    T dequeue(unsigned timeout_ms) {T t; return t;}

    CWnd* m_gui;
    UINT  m_message;
};


//////////////////////////////////////////////
// Template class implementation

template <class T>
ThreadQueue<T>::ThreadQueue()
{
}


template <class T>
ThreadQueue<T>::~ThreadQueue()
{
}


template <class T>
int ThreadQueue<T>::count()
{
    MutexLock lock(m_mutex);

    return m_list.size();
}



template <class T>
T ThreadQueue<T>::enqueue(T element)
{
    MutexLock lock(m_mutex);

    m_list.push_front(element);

    m_semaphore.signal();

    return element;
}



template <class T>
T ThreadQueue<T>::dequeue()
{
    bool rc = m_semaphore.wait();

    ITI_ASSERT(rc);

    MutexLock lock(m_mutex);

    ITI_ASSERT_GREATER(m_list.size(), (size_t)0);

    T element = m_list.back();
    m_list.pop_back();

    return element;
}


template <class T>
T ThreadQueue<T>::dequeue(unsigned timeout_ms)
{
    T element;

    if(m_semaphore.wait(timeout_ms))
    {
        MutexLock lock(m_mutex);

        ITI_ASSERT_GREATER(m_list.size(), (size_t)0);

        element = m_list.back();
        m_list.pop_back();
    }

    return element;
}







template <class T>
GuiMessagingThreadQueue<T>::GuiMessagingThreadQueue(CWnd* gui, UINT message) :
    m_gui(gui), m_message(message)
{
    ASSERT(gui);
    ASSERT(message);
}


    
template <class T>
T GuiMessagingThreadQueue<T>::enqueue(T element)
{
    ThreadQueue<T>::enqueue(element);
    ::PostMessage(m_gui->m_hWnd, m_message, 0, 0);

    return element;
}



template <class T>
T GuiMessagingThreadQueue<T>::dequeue()
{
    // Non-blocking
    return ThreadQueue<T>::dequeue(0);
}




#endif

