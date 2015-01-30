#pragma once


// FirmwareUpgradeDlg dialog
#include <resource.h>
#include "afxcmn.h"
#include "afxwin.h"

#include <ThreadQueue.h>
#include <Commands.h>
#include <FpgaComm.h>
#include <DialogEx.h>
#include <crc32.h>
#include <vector>


class FirmwareUpgradeDlg : public DialogEx
{
    class Exception : public ITI::Exception
    {
        public:
            inline Exception(const std::string& ex) : ITI::Exception(ex) {}
    };

    DECLARE_DYNAMIC(FirmwareUpgradeDlg)

public:
	FirmwareUpgradeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~FirmwareUpgradeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FIRMWARE_UPGRADE };

    enum UpgradeType { UpgradeFirmware, UpgradeFPGA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg LRESULT OnTransactionComplete(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonStart();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual INT_PTR DoModal(UpgradeType upgradeType);

private:
    enum
    {
        UPGRADE_TIMEOUT_ms = 5000,
        CLOSE_TIMEOUT_ms   = 5000,
    };

    void ResetState();
    void SendFileBegin(CString& filename);
    void SendFileNext();
    void HandleEndOfFile();
    unsigned SendNextFirmwareFileData();
    unsigned SendNextFPGAFileData();
    unsigned HexCharToValue(char c);
    unsigned GetNextHexValue(std::string& string, unsigned bytes, char* chksum = NULL);

    CProgressCtrl m_progressBar;
    CStatic       m_staticStatus;
    CButton       m_buttonStart;

    TransactionGuiMessagingThreadQueue::shared_ptr m_commRspQueue;

    UpgradeType m_upgradeType;
    FpgaComm&   m_comm;
    BOOL        m_running;
    BOOL        m_closePending;
    FILE*       m_upgradeFile;
    CString     m_upgradeFileName;
    unsigned    m_upgradeFileIndex;
    unsigned    m_upgradeFileLength;

    CRC32       m_upgradeFileCRC;

    time_t      m_startTime;

    std::vector<char> m_fileBuffer;
    unsigned          m_fileLineCount;
};
