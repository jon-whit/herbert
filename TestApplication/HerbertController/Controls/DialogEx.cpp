// DialogEx.cpp : implementation file
//

#include "stdafx.h"
#include "DialogEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogEx dialog


DialogEx::DialogEx(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(DialogEx)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}



/////////////////////////////////////////////////////////////////////////////
// DialogEx added functionality

int DialogEx::ShowMsgBox(UINT type, const char* title, const char* format, ...)
{
    CString msg;
    va_list argList;

    va_start(argList, format);
    msg.FormatV(format, argList);
    va_end(argList);

    return MessageBox(msg, title, type);
}


