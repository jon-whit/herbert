#if !defined(AFX_DIALOGEX_H__4E9D93FB_6522_45A7_AE07_AEFF7793CC37__INCLUDED_)
#define AFX_DIALOGEX_H__4E9D93FB_6522_45A7_AE07_AEFF7793CC37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogEx.h : header file
//




/////////////////////////////////////////////////////////////////////////////
// DialogEx dialog

class DialogEx : public CDialog
{
// Construction
public:
        DialogEx(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

// Interface
public:
    int  ShowMsgBox(UINT type, const char* title, const char* format, ...);
};



#endif // !defined(AFX_DIALOGEX_H__4E9D93FB_6522_45A7_AE07_AEFF7793CC37__INCLUDED_)
