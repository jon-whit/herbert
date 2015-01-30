// ReaderControlDlg.h : header file
//

#if !defined(AFX_READERCONTROLDLG_H__3A64BE89_8C70_48E1_9C1B_258123F9AFBF__INCLUDED_)
#define AFX_READERCONTROLDLG_H__3A64BE89_8C70_48E1_9C1B_258123F9AFBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <BtnST.h>
#include <DialogEx.h>
#include <FpgaComm.h>
#include <TabCtrlEx.h>
#include <ManualDlg.h>
#include <SystemDlg.h>
#include <PcbDlg.h>
#include <Instrument.h>
#include "afxwin.h"



/////////////////////////////////////////////////////////////////////////////
// CReaderControlDlg dialog

class CReaderControlDlg : public DialogEx
{
// Construction
public:
    CReaderControlDlg(CWnd* pParent = NULL);    // standard constructor
    ~CReaderControlDlg();                       // standard destructor

    virtual INT_PTR DoModal();

// Dialog Data
    enum { IDD = IDD_READERCONTROL_DIALOG };

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CReaderControlDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

public:
    void abort() { OnButtonStop(); }

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    virtual void OnCancel();
    afx_msg void OnButtonConnect();
    afx_msg void OnButtonStop();
    afx_msg LRESULT OnTransactionComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnInstrumentEvent(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    DECLARE_MESSAGE_MAP()

    void UpdateCommPorts();
    void UpdateCommPorts(const CString& fpgaPort, const CString& pcbPort);

    // Controls
    CButtonST  m_buttonStop;
    CButton    m_buttonConnect;
    CComboBox  m_comboFPGACommPort;
    CComboBox  m_comboPCBCommPort;
    TabCtrlEx  m_tabs;
    CStatic    m_staticFPGAFirmwareVersion;
    CStatic    m_staticFPGAFirmwareBuildDate;
    CStatic    m_staticFPGAVersion;
    CStatic    m_staticFPGAFirmwareVersionLabel;
    CStatic    m_staticFPGAFirmwareBuildDateLabel;
    CStatic    m_staticFPGAVersionLabel;
    CStatic    m_staticPCBFirmwareVersion;
    CStatic    m_staticPCBFirmwareBuildDate;
    CStatic    m_staticPCBFirmwareVersionLabel;
    CStatic    m_staticPCBFirmwareBuildDateLabel;
    CStatic    m_staticHDImageVersion;
    CStatic    m_staticSystemSerialNumber;


private:
    enum { FIRMWARE_POLL_PERIOD_ms = 2000 };





    void DisplayConnected();



    
    
    Instrument& m_instrument;

    GuiInstrumentEventQueue::shared_ptr m_instrumentEventQueue;



    int m_waitingForResponseCount;

    FpgaComm&   m_fpgaComm;

    TransactionGuiMessagingThreadQueue::shared_ptr m_rspQueue;

    // Tab dialogs
    ManualDlg         m_tabDlgManual;
    PcbDlg            m_tabDlgPcb;
    SystemDlg         m_tabDlgSystem;

    CArray<ReaderControlTabDlg*, ReaderControlTabDlg*> m_tabDialogs;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READERCONTROLDLG_H__3A64BE89_8C70_48E1_9C1B_258123F9AFBF__INCLUDED_)
