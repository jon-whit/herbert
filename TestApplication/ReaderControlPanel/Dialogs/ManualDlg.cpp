// ManualDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderControl.h"
#include "ManualDlg.h"
#include <FirmwareUpgradeDlg.h>

#include <Messages.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const static char* profile_sectionManual     = "Manual Settings";
const static char* profile_keyAutoHeader     = "auto header";
const static int   profile_defaultAutoHeader = TRUE;
const static char* profile_keyMsg1           = "msg1";



/////////////////////////////////////////////////////////////////////////////
// ManualDlg dialog


ManualDlg::ManualDlg(CWnd* parentWnd)
	: ReaderControlTabDlg(ManualDlg::IDD, parentWnd)
    , m_comm(FpgaComm::getInstance())
    , m_connected(false)
	, m_debugCaptureDlg(NULL)
    , m_activeEditBox(NULL)
    , m_infoRspQueue(new InfoResponseGuiMessagingThreadQueue(this, WM_COMM_INFO_RESPONSE))
    , m_pendingRspQueue(new PendingResponseGuiMessagingThreadQueue(this, WM_COMM_PENDING_RESPONSE))
    , m_rspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
{
    //{{AFX_DATA_INIT(ManualDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ManualDlg::DoDataExchange(CDataExchange* pDX)
{
	ReaderControlTabDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ManualDlg)
	DDX_Control(pDX, IDC_EDIT_MSG1, m_editMsg1);
	DDX_Control(pDX, IDC_BUTTON_SEND1, m_buttonSend1);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputWindow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ManualDlg, ReaderControlTabDlg)
	//{{AFX_MSG_MAP(ManualDlg)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_OUTPUT, OnButtonClearOutput)
	ON_EN_SETFOCUS(IDC_EDIT_MSG1, OnSetFocusEditMsg1)
	ON_BN_CLICKED(IDC_BUTTON_SEND1, OnButtonSend1)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_COMM_INFO_RESPONSE, OnInfoResponse)
    ON_MESSAGE(WM_COMM_PENDING_RESPONSE, OnPendingResponse)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnTransactionComplete)
	ON_BN_CLICKED(IDC_BUTTON_UPGRADE_FIRMWARE, &ManualDlg::OnBnClickedButtonUpgradeFirmware)
	ON_BN_CLICKED(IDC_BUTTON_UPGRADE_FPGA, &ManualDlg::OnBnClickedButtonUpgradeFpga)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_CAPTURE, &ManualDlg::OnBnClickedButtonDebugCapture)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ManualDlg message handlers

BOOL ManualDlg::OnInitDialog() 
{
	ReaderControlTabDlg::OnInitDialog();
	
    int autoHeader = AfxGetApp()->GetProfileInt(profile_sectionManual, profile_keyAutoHeader, profile_defaultAutoHeader);

    m_editMsg1.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg1));

    SetTimer(1, TIMER_PERIOD_ms, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void ManualDlg::OnOK()
{
    if(m_activeEditBox)
    {
        GetAndSendCmd(m_activeEditBox);
    }
}


void ManualDlg::OnAbort()
{
}


BOOL ManualDlg::SaveSettingsAndExit()
{
    AfxGetApp()->WriteProfileInt(profile_sectionManual, profile_keyAutoHeader, profile_defaultAutoHeader);

    CString msg;
    m_editMsg1.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg1, msg);

    return TRUE;
}


void ManualDlg::DisplayFPGAConnected(BOOL connected)
{
    m_outputWindow.EnableWindow(connected);
    m_editMsg1.EnableWindow(connected);
    m_buttonSend1.EnableWindow(connected);

    m_connected = connected;
}



void ManualDlg::OnTimer(UINT nIDEvent) 
{
    KillTimer(nIDEvent);
	
	ReaderControlTabDlg::OnTimer(nIDEvent);
}



LRESULT ManualDlg::OnInfoResponse(WPARAM wParam, LPARAM lParam)
{
    Commands::InfoResponse::shared_ptr infoResponse(m_infoRspQueue->dequeue());

    if(infoResponse)
    {
        m_outputWindow.SetSel(0, -1);
        m_outputWindow.SetSel(-1, -1);

        m_outputWindow.ReplaceSel(infoResponse->infoRsp()->GetFullPktStr() + " -- Info Response\r\n");
    }

    return 0;
}


LRESULT ManualDlg::OnPendingResponse(WPARAM wParam, LPARAM lParam)
{
    Commands::PendingResponse::shared_ptr pendingResponse(m_pendingRspQueue->dequeue());

    if(pendingResponse)
    {
        m_outputWindow.SetSel(0, -1);
        m_outputWindow.SetSel(-1, -1);

        m_outputWindow.ReplaceSel(pendingResponse->pendingRsp()->GetFullPktStr() + " -- Pending Response\r\n");
    }

    return 0;
}


LRESULT ManualDlg::OnTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_rspQueue->dequeue());

    if(transaction)
    {
        if(transaction->validRsp() || transaction->status().GetLength())
        {
            CString rspPktString = transaction->rspPktString();
            if(transaction->isOfflineRsp())
            {
                rspPktString += " -- Completed Response";
            }

            m_outputWindow.SetSel(0, -1);
            m_outputWindow.SetSel(-1, -1);
            m_outputWindow.ReplaceSel(rspPktString + "\r\n");
        }
        else
        {
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "'%s'", transaction->getErrorString());
        }
    }

    return 0;
}

void ManualDlg::OnButtonClearOutput() 
{
    m_outputWindow.SetWindowText("");
}


void ManualDlg::SendCmd(CString& cmd)
{
    try
    {
        if(cmd.ReverseFind(CommPkt::START_OF_FRAME) >= 0 ||
           cmd.Find(CommPkt::END_OF_FRAME) >= 0)
        {
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Invalid Pkt", "Invalid Pkt Structure\n'%s'", cmd);
            return;
        }

        cmd.TrimLeft();
        cmd.TrimRight();

        Commands::Transaction::shared_ptr command(new Commands::FpgaManual(cmd, profile_defaultAutoHeader));
        m_comm.sendCommand(m_infoRspQueue, m_pendingRspQueue, m_rspQueue, command);
    }
    catch(CommPkt::ExceptionInvalidPkt& ex)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Invalid Pkt", "%s", ex.what()); 
    }
    catch(CommPkt::Exception& ex)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Invalid Pkt", "%s", ex.what()); 
    }
}


void ManualDlg::OnSetFocusEditMsg1() 
{
    m_activeEditBox = &m_editMsg1;
}


void ManualDlg::GetAndSendCmd(CEdit* editMsg)
{
    CString cmd;
    editMsg->GetWindowText(cmd);
    if(cmd.GetLength()) SendCmd(cmd);
}


void ManualDlg::OnButtonSend1() 
{
    GetAndSendCmd(&m_editMsg1);
}


void ManualDlg::OnBnClickedButtonUpgradeFirmware()
{
	FirmwareUpgradeDlg dlg(m_pParentWnd);
    dlg.DoModal(FirmwareUpgradeDlg::UpgradeFirmware);
}


void ManualDlg::OnBnClickedButtonUpgradeFpga()
{
	FirmwareUpgradeDlg dlg(m_pParentWnd);
    dlg.DoModal(FirmwareUpgradeDlg::UpgradeFPGA);
}


void ManualDlg::OnBnClickedButtonDebugCapture()
{
	if(m_debugCaptureDlg)
    {
        m_debugCaptureDlg->SetActiveWindow();
    }
    else
    {
        m_debugCaptureDlg = new DebugCaptureDlg(this);
        m_debugCaptureDlg->Create();
    }
}

void ManualDlg::CloseDebugCaptureDlg()
{
    if(m_debugCaptureDlg)
    {
        // Note: the Debug Capture dialog will delete
        //       itself, just hide is and clear the
        //       pointer here.
        m_debugCaptureDlg->ShowWindow(SW_HIDE);
        m_debugCaptureDlg = NULL;
    }
}