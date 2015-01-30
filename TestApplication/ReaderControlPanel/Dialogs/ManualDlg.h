#if !defined(AFX_MANUALDLG_H__26479D15_5C86_4955_A804_C858877F2E60__INCLUDED_)
#define AFX_MANUALDLG_H__26479D15_5C86_4955_A804_C858877F2E60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ManualDlg.h : header file
//

#include <TabCtrlEx.h>
#include <ThreadQueue.h>
#include <Commands.h>
#include <FpgaComm.h>
#include <ReaderControlTabDlg.h>


/////////////////////////////////////////////////////////////////////////////
// ManualDlg dialog

class ManualDlg : public ReaderControlTabDlg
{
// Construction
public:
	ManualDlg(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(ManualDlg)
	enum { IDD = IDD_DIALOG_MANUAL };
	CButton	m_buttonSend1;
	CButton	m_buttonSend2;
	CButton	m_buttonSend3;
	CButton	m_buttonSend4;
	CButton	m_buttonSend5;
	CButton	m_buttonSend6;
	CButton	m_buttonSend7;
	CButton	m_buttonSend8;
	CButton	m_buttonSend9;
	CButton	m_buttonSend10;
	CButton	m_buttonSend11;
	CButton	m_buttonSend12;
	CEdit	m_editMsg1;
	CEdit	m_editMsg2;
	CEdit	m_editMsg3;
	CEdit	m_editMsg4;
	CEdit	m_editMsg5;
	CEdit	m_editMsg6;
	CEdit	m_editMsg7;
	CEdit	m_editMsg8;
	CEdit	m_editMsg9;
	CEdit	m_editMsg10;
	CEdit	m_editMsg11;
	CEdit	m_editMsg12;
	CEdit	m_outputWindow;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ManualDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


    
protected:
	// Generated message map functions
	//{{AFX_MSG(ManualDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonClearOutput();
	afx_msg void OnSetFocusEditMsg1();
	afx_msg void OnSetFocusEditMsg2();
	afx_msg void OnSetFocusEditMsg3();
	afx_msg void OnSetFocusEditMsg4();
	afx_msg void OnSetFocusEditMsg5();
	afx_msg void OnSetFocusEditMsg6();
	afx_msg void OnSetFocusEditMsg7();
	afx_msg void OnSetFocusEditMsg8();
	afx_msg void OnSetFocusEditMsg9();
	afx_msg void OnSetFocusEditMsg10();
	afx_msg void OnSetFocusEditMsg11();
	afx_msg void OnSetFocusEditMsg12();
	afx_msg void OnButtonSend1();
	afx_msg void OnButtonSend2();
	afx_msg void OnButtonSend3();
	afx_msg void OnButtonSend4();
	afx_msg void OnButtonSend5();
	afx_msg void OnButtonSend6();
	afx_msg void OnButtonSend7();
	afx_msg void OnButtonSend8();
	afx_msg void OnButtonSend9();
	afx_msg void OnButtonSend10();
	afx_msg void OnButtonSend11();
	afx_msg void OnButtonSend12();
	afx_msg void OnButtonClearCmds();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
    afx_msg LRESULT OnInfoResponse(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPendingResponse(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTransactionComplete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


// Implementation
public:
    BOOL SaveSettingsAndExit();
    void DisplayFPGAConnected(BOOL connected);
    void DisplayPCBConnected(BOOL connected) {}
    void OnAbort();

    
private:
    enum { TIMER_PERIOD_ms = 50 };

    void SendCmd(CString& cmd);
    void GetAndSendCmd(CEdit* editMsg);

    
    CEdit* m_activeEditBox;

    InfoResponseGuiMessagingThreadQueue::shared_ptr    m_infoRspQueue;
    PendingResponseGuiMessagingThreadQueue::shared_ptr m_pendingRspQueue;
    TransactionGuiMessagingThreadQueue::shared_ptr     m_rspQueue;

    FpgaComm& m_comm;

    BOOL  m_connected;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANUALDLG_H__26479D15_5C86_4955_A804_C858877F2E60__INCLUDED_)
