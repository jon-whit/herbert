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
#include <DebugCaptureDlg.h>
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
	CEdit	m_editMsg1;
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
	afx_msg void OnButtonSend1();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
    afx_msg LRESULT OnInfoResponse(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPendingResponse(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTransactionComplete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


	DebugCaptureDlg*            m_debugCaptureDlg;

// Implementation
public:
    BOOL SaveSettingsAndExit();
    void DisplayFPGAConnected(BOOL connected);
    void DisplayPCBConnected(BOOL connected) {}
    void OnAbort();
	void CloseDebugCaptureDlg();

    
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
public:
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButtonUpgradeFirmware();
	afx_msg void OnBnClickedButtonUpgradeFpga();
	afx_msg void OnBnClickedButtonDebugCapture();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANUALDLG_H__26479D15_5C86_4955_A804_C858877F2E60__INCLUDED_)
