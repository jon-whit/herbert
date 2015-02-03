// HerbertControllerTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HerbertControllerTabDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HerbertControllerTabDlg dialog


HerbertControllerTabDlg::HerbertControllerTabDlg(UINT nIDTemplate, CWnd* pParent)
	: TabCtrlExDlg(nIDTemplate, pParent)
{
    ITI_ASSERT_NOT_NULL(pParent);
	//{{AFX_DATA_INIT(HerbertControllerTabDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void HerbertControllerTabDlg::DoDataExchange(CDataExchange* pDX)
{
	DialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(HerbertControllerTabDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(HerbertControllerTabDlg, TabCtrlExDlg)
	//{{AFX_MSG_MAP(HerbertControllerTabDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

