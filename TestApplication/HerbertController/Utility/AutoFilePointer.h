///////////////////////////////////////////////////////
//
//  AutoFilePointer.h
//
//  Simple class to file pointers - mainly to ensure
//  files are closed.
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert



#ifndef AutoFilePointer_h
#define AutoFilePointer_h

#include <stdio.h>
#include <Exceptions.h>



class AutoFilePointer
{
public:
    AutoFilePointer()        : m_f(NULL), m_own(false) {}
    AutoFilePointer(FILE* f) : m_f(f), m_own(true)     {}
    AutoFilePointer(AutoFilePointer& afp) :
        m_f(afp.m_f), m_own(afp.m_own)                 {afp.m_own = false;}
    ~AutoFilePointer()                                 {close();}
    inline operator FILE*()                            {return m_f;}
    inline FILE* operator->()                          {ITI_ASSERT(m_f); return m_f;}
    inline FILE* release()                             {m_own = false; return m_f;}
    inline FILE* operator=(AutoFilePointer& afp)       {if (&afp != this) { close(); m_f = afp.m_f; m_own = afp.m_own; afp.m_own = false; return m_f; }}
    inline FILE* operator=(FILE* f)                    {if (m_f != f) { close(); } m_f = f; m_own = true; return m_f;}
private:
    void close()                                       {if(m_f && m_own) fclose(m_f);}

    FILE* m_f;
    bool  m_own;
};


#endif
