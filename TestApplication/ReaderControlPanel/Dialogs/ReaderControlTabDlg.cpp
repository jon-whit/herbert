// ReaderControlTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderControlTabDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ReaderControlTabDlg dialog


ReaderControlTabDlg::ReaderControlTabDlg(UINT nIDTemplate, CWnd* pParent)
	: TabCtrlExDlg(nIDTemplate, pParent)
{
    ITI_ASSERT_NOT_NULL(pParent);
	//{{AFX_DATA_INIT(ReaderControlTabDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void ReaderControlTabDlg::DoDataExchange(CDataExchange* pDX)
{
	DialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ReaderControlTabDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ReaderControlTabDlg, TabCtrlExDlg)
	//{{AFX_MSG_MAP(ReaderControlTabDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

