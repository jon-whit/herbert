// customTabControl.cpp : implementation file
//

#include "TabCtrlEx.h"



enum TabOffsets
{
    offsetLeft     =  3,
    offsetRight    = -3,
    offsetTop      =  27,
    offsetBottom   = -3
};




/////////////////////////////////////////////////////////////////////////////
// TabCtrlEx

TabCtrlEx::TabCtrlEx()
{
}



TabCtrlEx::~TabCtrlEx()
{
}



void TabCtrlEx::changeTab(int tab)
{
    if(tab >= GetItemCount() && tab < 0)
    {
        throw IndexException("Requested Tab Out Of Range");
    }

    for(int i = 0; i < GetItemCount(); ++i)
    {
        m_tabDialogs[i]->ShowWindow(SW_HIDE);
    }

    m_tabDialogs[tab]->ShowWindow(SW_SHOW);

    SetCurSel(tab);
}



void TabCtrlEx::addDialog(TabCtrlExDlg& newDialog, CString dialogTitle, bool center)
{
    insertDialog(newDialog, dialogTitle, GetItemCount(), center);
}



void TabCtrlEx::insertDialog(TabCtrlExDlg& newDialog, CString dialogTitle, int tab, bool center)
{
    RECT tabRect;
    RECT dlgRect;

    int  count   = GetItemCount();
    
    if( tab < 0 )
    {
        throw IndexException("Requested Tab Out Of Range");
    }

    InsertItem(tab, dialogTitle);
    m_tabDialogs.InsertAt(tab, &newDialog);

    newDialog.Create(newDialog.GetTemplateId(), this);

    GetClientRect(&tabRect);

    tabRect.left   += offsetLeft;
    tabRect.right  += offsetRight;
    tabRect.top    += offsetTop;
    tabRect.bottom += offsetBottom;

    // Center window
    if(center)
    {
        newDialog.GetClientRect(&dlgRect);

        if(dlgRect.right - dlgRect.left < tabRect.right - tabRect.left)
        {
            tabRect.left += ((tabRect.right - tabRect.left) - (dlgRect.right - dlgRect.left)) / 2;
        }
        else
        {
            TRACE("Warning: Dialog '%s' horizontal size is larger than the tab control\n", dialogTitle); 
        }

        if(dlgRect.bottom - dlgRect.top < tabRect.bottom - tabRect.top)
        {
            tabRect.top += ((tabRect.bottom - tabRect.top) - (dlgRect.bottom - dlgRect.top)) / 2;
        }
        else
        {
            TRACE("Warning: Dialog '%s' virtical  size is larger than the tab control\n", dialogTitle); 
        }
    }

    newDialog.MoveWindow(&tabRect, true);

    if( count == 0 )
    {
        newDialog.ShowWindow(SW_SHOW);
        m_tabDialogs[GetCurSel()]->RedrawWindow();
    }
    else
    {
        newDialog.ShowWindow(SW_HIDE);
        m_tabDialogs[GetCurSel()]->RedrawWindow();
    }

    RedrawWindow();
}



void TabCtrlEx::removeDialog(int tab)
{
    if( tab >= 0 && tab < m_tabDialogs.GetSize() )
    {
        DeleteItem(tab);
        m_tabDialogs.RemoveAt(tab);

        if( m_tabDialogs.GetSize() )
        {
            m_tabDialogs[GetCurSel()]->ShowWindow(SW_SHOW);
            m_tabDialogs[GetCurSel()]->RedrawWindow();
        }

        RedrawWindow();
    }
    else
    {
        throw IndexException("Requested Tab Out Of Range");
    }
}



TabCtrlExDlg* TabCtrlEx::getDialog(int page)
{
    if( page < GetItemCount() && page >= 0 )
    {
        return m_tabDialogs[page];
    }
    else
    {
        return NULL;
    }
}



BEGIN_MESSAGE_MAP(TabCtrlEx, CTabCtrl)
    //{{AFX_MSG_MAP(TabCtrlEx)
    ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// TabCtrlEx message handlers

void TabCtrlEx::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
    changeTab(GetCurSel());

    *pResult = 0;
}







/////////////////////////////////////////////////////////////////////////////
// TabCtrlExDlg dialog


TabCtrlExDlg::TabCtrlExDlg(UINT nIDTemplate, CWnd* pParent)
	: DialogEx(nIDTemplate, pParent)
    , m_templateId(nIDTemplate)
{
    ASSERT(pParent);
    ASSERT(nIDTemplate);
	//{{AFX_DATA_INIT(TabCtrlExDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BOOL TabCtrlExDlg::OnInitDialog()
{
    DialogEx::OnInitDialog();

    EnableThemeDialogTexture(m_hWnd, ETDT_ENABLETAB);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}




void TabCtrlExDlg::DoDataExchange(CDataExchange* pDX)
{
	DialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TabCtrlExDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TabCtrlExDlg, DialogEx)
	//{{AFX_MSG_MAP(TabCtrlExDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void TabCtrlExDlg::OnOK() 
{
    // Override default button (OK)
}

void TabCtrlExDlg::OnCancel() 
{
    // Override escape
}




// EOF
