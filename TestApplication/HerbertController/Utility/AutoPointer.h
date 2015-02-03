/////////////////////////////////////////////////////////////
//
//  AutoPointer.h
//
//  Generic Auto Pointer
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef AutoPointer_h
#define AutoPointer_h



template <class T>
class AutoPointer
{
public:
    AutoPointer()                   : m_ptr(NULL),     m_ownPtr(FALSE)            {}
    AutoPointer(T* ptr)             : m_ptr(ptr),      m_ownPtr(ptr != NULL)      {}
    AutoPointer(AutoPointer<T>& ap) : m_ptr(ap.m_ptr), m_ownPtr(ap.m_ptr != NULL) { ap.m_ownPtr = FALSE; }

    ~AutoPointer() {if(m_ptr && m_ownPtr) delete m_ptr;}

    AutoPointer<T>& operator=(const AutoPointer<T>& ap) { m_ptr = ap.m_ptr; m_ownPtr = TRUE; ap.m_ownPtr = FALSE; } 
    T* operator=(T* p)                                  { m_ptr = p; m_ownPtr = TRUE; return p; } 

    T* get()        {return m_ptr;}

    operator T*()   {return m_ptr;}
    T& operator *() {return m_ptr;}
    T* operator->() {return m_ptr;}
    T* releasePointer() { T* ptr = m_ptr; m_ptr = NULL; return ptr;}

private:

    BOOL m_ownPtr;
    T*   m_ptr;
};

#endif
