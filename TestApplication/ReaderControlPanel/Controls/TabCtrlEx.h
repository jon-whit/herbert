// customTabControl.h : header file
//

#ifndef customTabControl_h
#define customTabControl_h

#include "stdafx.h"
#include <afxtempl.h>
#include <stdexcept>
#include <DialogEx.h>
#include <Exceptions.h>


/////////////////////////////////////////////////////////////////////////////
// TabCtrlExDlg dialog

class TabCtrlExDlg : public DialogEx
{
// Construction
public:
    TabCtrlExDlg(UINT nIDTemplate, CWnd* pParent);   // standard constructor

    UINT GetTemplateId() {return m_templateId;}

    // Pure virtual functions to assure they are implemented in
    // derived classes
    virtual BOOL SaveSettingsAndExit() = 0;

    // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ManualProtocolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TabCtrlExDlg)
	virtual void OnOK();
	virtual void OnCancel();
    virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Interface
protected:
    UINT m_templateId;
};






/////////////////////////////////////////////////////////////////////////////
// TabCtrlEx window

class TabCtrlEx : public CTabCtrl
{
    //A custom tab control. This class allows the switching of
    //dialogs during the page changing.

    public:
        class IndexException : public ITI::Exception
        {
        public:
            inline IndexException(const char* ex) : ITI::Exception(ex) {};
        };



    // Construction
    public:
        TabCtrlEx();

    // Attributes
    public:

    // Operations
    public:
        void addDialog(TabCtrlExDlg& newDialog, CString dialogTitle, bool center = true);
        void insertDialog(TabCtrlExDlg& newDialog, CString dialogTitle, int tab, bool center = true);
        void removeDialog(int tab);

        void changeTab(int tab);

        TabCtrlExDlg* getDialog(int tab);
        inline TabCtrlExDlg* getCurrentDialog() { return getDialog(GetCurSel()); };



    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TabCtrlEx)
    //}}AFX_VIRTUAL

    // Implementation
    public:
        virtual ~TabCtrlEx();

        // Generated message map functions
    protected:
        //{{AFX_MSG(TabCtrlEx)
        afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
        //}}AFX_MSG

        DECLARE_MESSAGE_MAP()

    private:
        CArray<TabCtrlExDlg*, TabCtrlExDlg*> m_tabDialogs;
};



#endif
