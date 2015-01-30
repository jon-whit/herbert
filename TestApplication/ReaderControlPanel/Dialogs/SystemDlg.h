#pragma once

#include "afxwin.h"
#include <ReaderControlTabDlg.h>
#include <PcbComm.h>
#include <resource.h>
#include <DebugCaptureDlg.h>
#include <Instrument.h>
#include <FontCtrl.h>

// SystemDlg dialog 

class SystemDlg : public ReaderControlTabDlg
{
	DECLARE_DYNAMIC(SystemDlg)

public:
	SystemDlg(CWnd* pParent);
	virtual ~SystemDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SYSTEM };


    BOOL SaveSettingsAndExit();
    void DisplayFPGAConnected(BOOL connected);
    void DisplayPCBConnected(BOOL connected);
    void OnAbort();

    void CloseDeviceTestDlg();
    void CloseSelfTestDlg();
    void CloseFileNamingToolDlg();
    void CloseDebugCaptureDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    afx_msg LRESULT OnTransactionComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRachisConfigUpdate(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

    afx_msg void OnBnClickedButtonUpgradeFirmware();
    afx_msg void OnBnClickedButtonUpgradeFpga();
    afx_msg void OnBnClickedButtonCalibration();
    afx_msg void OnBnClickedButtonInitSystem();
    afx_msg void OnBnClickedButtonOpenDoor();
    afx_msg void OnBnClickedButtonCloseDoor();
    afx_msg void OnBnClickedButtonSelfTest();
    afx_msg void OnBnClickedButtonDeviceTest();
    afx_msg void OnBnClickedButtonDebugCapture();
    afx_msg void OnBnClickedCheckMonitorConnection();
    afx_msg void OnBnClickedButtonSaveConfiguration();
    afx_msg void OnBnClickedButtonFileNamingTool();

    void SetIdle();

    void ShowUnconfigured(const std::string& errorMsg);
    void UpdateSaveConfigurationButton();

    FpgaComm&                                                        m_fpgaComm;
    PcbComm&                                                         m_pcbComm;
    TransactionGuiMessagingThreadQueue::shared_ptr                   m_commRspQueue;

    CButton m_buttonUpgradeFirmware;
    CButton m_buttonUpgradeFPGA;
    CButton m_buttonCalibration;
    CButton m_buttonInitSystem;
    CButton m_buttonOpenDoor;
    CButton m_buttonCloseDoor;
    CButton m_buttonRunSelfTest;
    CButton m_buttonDeviceTest;
    CButton m_buttonDebugCapture;
    CStatic m_staticRachisConfigFrame;
    CButton m_checkMonitorConnection;
    CButton m_buttonFileNamingTool;
    CFontCtrl<CStatic> m_staticPcrMaskConfiguration;
    CFontCtrl<CStatic> m_staticLogLevel;
    CFontCtrl<CStatic> m_staticRunImageCaptureType;
    CFontCtrl<CStatic> m_staticDataRetentionTime;
    CFontCtrl<CStatic> m_staticGeoCorrection;
    CFontCtrl<CStatic> m_staticShutdownTime;
    CButton m_buttonSaveConfiguration;

    CArray<CWnd*, CWnd*>        m_dlgControls;

    DebugCaptureDlg*            m_debugCaptureDlg;

    bool                        m_monitorConnection;
};
