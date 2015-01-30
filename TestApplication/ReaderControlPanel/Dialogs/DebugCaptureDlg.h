#pragma once

#include "resource.h"
#include <DialogEx.h>
#include <Commands.h>
#include <FpgaComm.h>
#include <ValidatingEdit.h>
#include "afxwin.h"
#include "afxcmn.h"


class DebugCaptureDlg : public DialogEx
{
    DECLARE_DYNAMIC(DebugCaptureDlg)

public:
    DebugCaptureDlg(CWnd* parent);
    virtual ~DebugCaptureDlg();
    virtual BOOL Create();
    virtual BOOL OnInitDialog();
    virtual void OnCancel();

    enum { IDD = IDD_DIALOG_DEBUG_CAPTURE };

protected:
    enum
    {
        defaultByteCount = 1000,
        maxByteCount     = 1000000
    };

    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    virtual INT_PTR DoModal();
    virtual void PostNcDestroy();
    virtual void OnOK();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonCaptureDebugOutput();
    afx_msg void OnBnClickedButtonSave();

    LRESULT OnCommTransactionComplete(WPARAM wParam, LPARAM lParam);

    void SetStateCapturing();
    void SetStateIdle();
    void SetStateError(const CString& error);
    void RequestDebugOutput();
    void SaveOutputToFile(const CString& filename);

    CEdit         m_editDebugOutput;
    CButton       m_buttonCaptureDebug;
    CButton       m_buttonSave;
    UnsignedEdit  m_editByteCount;
    CProgressCtrl m_progressDownload;

    CWnd*     m_parentWindow;
    FpgaComm& m_comm;

    TransactionGuiMessagingThreadQueue::shared_ptr m_rspQueue;

    unsigned m_remainingBytesToCapture;
    unsigned m_captureOffset;
    CString  m_data;
    bool     m_capturing;
};
