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
#include <HerbertControllerTabDlg.h>


/////////////////////////////////////////////////////////////////////////////
// ManualDlg dialog

class ManualDlg : public HerbertControllerTabDlg
{
// Construction
public:
	ManualDlg(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(ManualDlg)
	enum { IDD = IDD_DIALOG_MANUAL };
	CButton	m_buttonSend1;
	CButton m_buttonSend2;
	CEdit	m_editMsg1;
	CEdit	m_editMsg2;
	CEdit	m_outputWindow;

	CButton m_buttonLIn;
	CButton m_buttonLOut;
	CButton m_buttonLC;
	CButton m_buttonLCC;
	CButton m_buttonL2;
	CButton m_buttonFIn;
	CButton m_buttonFOut;
	CButton m_buttonFC;
	CButton m_buttonFCC;
	CButton m_buttonF2;
	CButton m_buttonRIn;
	CButton m_buttonROut;
	CButton m_buttonRC;
	CButton m_buttonRCC;
	CButton m_buttonR2;
	CButton m_buttonBIn;
	CButton m_buttonBOut;
	CButton m_buttonBC;
	CButton m_buttonBCC;
	CButton m_buttonB2;
	CButton m_buttonUIn;
	CButton m_buttonUOut;
	CButton m_buttonUC;
	CButton m_buttonUCC;
	CButton m_buttonU2;
	CButton m_buttonDIn;
	CButton m_buttonDOut;
	CButton m_buttonDC;
	CButton m_buttonDCC;
	CButton m_buttonD2;

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
	afx_msg void OnEnSetfocusEditMsg2();
	afx_msg void OnBnClickedButtonSend2();
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
	afx_msg void OnBnClickedButtonLin();
	afx_msg void OnBnClickedButtonLout();
	afx_msg void OnBnClickedButtonLc();
	afx_msg void OnBnClickedButtonLcc();
	afx_msg void OnBnClickedButtonL2();
	afx_msg void OnBnClickedButtonFin();
	afx_msg void OnBnClickedButtonFout();
	afx_msg void OnBnClickedButtonFc();
	afx_msg void OnBnClickedButtonFcc();
	afx_msg void OnBnClickedButtonF2();
	afx_msg void OnBnClickedButtonRin();
	afx_msg void OnBnClickedButtonRout();
	afx_msg void OnBnClickedButtonRc();
	afx_msg void OnBnClickedButtonRcc();
	afx_msg void OnBnClickedButtonR2();
	afx_msg void OnBnClickedButtonBin();
	afx_msg void OnBnClickedButtonBout();
	afx_msg void OnBnClickedButtonBc();
	afx_msg void OnBnClickedButtonBcc();
	afx_msg void OnBnClickedButtonB2();
	afx_msg void OnBnClickedButtonUin();
	afx_msg void OnBnClickedButtonUout();
	afx_msg void OnBnClickedButtonUc();
	afx_msg void OnBnClickedButtonUcc();
	afx_msg void OnBnClickedButtonU2();
	afx_msg void OnBnClickedButtonDin();
	afx_msg void OnBnClickedButtonDout();
	afx_msg void OnBnClickedButtonDc();
	afx_msg void OnBnClickedButtonDcc();
	afx_msg void OnBnClickedButtonD2();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANUALDLG_H__26479D15_5C86_4955_A804_C858877F2E60__INCLUDED_)
