#pragma once

#include <TabCtrlEx.h>
#include <XListCtrl.h>
#include <ThreadQueue.h>
#include <Commands.h>
#include <PcbComm.h>
#include <ReaderControlTabDlg.h>
#include <resource.h>
#include "afxwin.h"
#include "afxcmn.h"
#include <ColorCtrl.h>

// PcbDlg dialog

class PcbDlg : public ReaderControlTabDlg
{
    DECLARE_DYNAMIC(PcbDlg)

public:
    PcbDlg(CWnd* pParent);
    virtual ~PcbDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_PCB };


public:
    BOOL SaveSettingsAndExit();
    void DisplayFPGAConnected(BOOL connected);
    void DisplayPCBConnected(BOOL connected);
    void OnAbort();


protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()


private:
	virtual void OnOK();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedButtonSend1();
    afx_msg void OnBnClickedButtonSend2();
    afx_msg void OnBnClickedButtonSend3();
    afx_msg void OnBnClickedButtonSend4();
    afx_msg void OnBnClickedButtonClearOutput();
    afx_msg void OnBnClickedButtonClearCmds();
    afx_msg LRESULT OnManualTransactionComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCmdTransactionComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCheckbox(WPARAM nItem, LPARAM nSubItem);
    afx_msg void OnBnClickedCheckPollStatus();

    void GetAndSendManualCmd(CEdit* editMsg);
    void SendManualCmd(CString& cmd);
    void SendCmd(Commands::Transaction::shared_ptr command);
    void SendNextCmd();
    void SetUpSystemStatus();
    void GetSystemStatus();
    void GetOnStates();
    void UpdateOnStateDisplay(int row, BOOL on);
    void UpdateVoltageDisplay(int row, const CString& value);
    void UpdateErrorDisplay(int row, BOOL error, const CString& value);

    CEdit      m_editOutputWindow;
    CEdit      m_editMsg1;
    CEdit      m_editMsg2;
    CEdit      m_editMsg3;
    CEdit      m_editMsg4;
    CButton    m_buttonSend1;
    CButton    m_buttonSend2;
    CButton    m_buttonSend3;
    CButton    m_buttonSend4;
    CXListCtrl m_listStatus;
    CButton    m_checkPollSystemStatus;
    CColorCtrl<CStatic> m_staticPwrDownRequested;

private:
    PcbComm&                                         m_pcbComm;
    TransactionGuiMessagingThreadQueue::shared_ptr   m_manualRspQueue;
    TransactionGuiMessagingThreadQueue::shared_ptr   m_cmdRspQueue;
    std::list<Commands::Transaction::shared_ptr>     m_pendingInfoCommands;
    std::list<Commands::Transaction::shared_ptr>     m_pendingStatusCommands;
};
