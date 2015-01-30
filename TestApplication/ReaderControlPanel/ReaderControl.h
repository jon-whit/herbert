// ReaderControl.h : main header file for the READERCONTROL application
//

#if !defined(AFX_READERCONTROL_H__7222D88C_92E2_426F_BB10_18E728C55CD2__INCLUDED_)
#define AFX_READERCONTROL_H__7222D88C_92E2_426F_BB10_18E728C55CD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CReaderControlApp:
// See ReaderControl.cpp for the implementation of this class
//

class CReaderControlApp : public CWinApp
{
public:
    CReaderControlApp();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CReaderControlApp)
    public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

// Implementation

    //{{AFX_MSG(CReaderControlApp)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READERCONTROL_H__7222D88C_92E2_426F_BB10_18E728C55CD2__INCLUDED_)
