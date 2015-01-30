// ManualDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderControl.h"
#include "ManualDlg.h"

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
const static char* profile_keyMsg2           = "msg2";
const static char* profile_keyMsg3           = "msg3";
const static char* profile_keyMsg4           = "msg4";
const static char* profile_keyMsg5           = "msg5";
const static char* profile_keyMsg6           = "msg6";
const static char* profile_keyMsg7           = "msg7";
const static char* profile_keyMsg8           = "msg8";
const static char* profile_keyMsg9           = "msg9";
const static char* profile_keyMsg10          = "msg10";
const static char* profile_keyMsg11          = "msg11";
const static char* profile_keyMsg12          = "msg12";




/////////////////////////////////////////////////////////////////////////////
// ManualDlg dialog


ManualDlg::ManualDlg(CWnd* parentWnd)
	: ReaderControlTabDlg(ManualDlg::IDD, parentWnd)
    , m_comm(FpgaComm::getInstance())
    , m_connected(false)
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
	DDX_Control(pDX, IDC_EDIT_MSG2, m_editMsg2);
	DDX_Control(pDX, IDC_EDIT_MSG3, m_editMsg3);
	DDX_Control(pDX, IDC_EDIT_MSG4, m_editMsg4);
	DDX_Control(pDX, IDC_EDIT_MSG5, m_editMsg5);
	DDX_Control(pDX, IDC_EDIT_MSG6, m_editMsg6);
	DDX_Control(pDX, IDC_EDIT_MSG7, m_editMsg7);
	DDX_Control(pDX, IDC_EDIT_MSG8, m_editMsg8);
	DDX_Control(pDX, IDC_EDIT_MSG9, m_editMsg9);
	DDX_Control(pDX, IDC_EDIT_MSG10, m_editMsg10);
	DDX_Control(pDX, IDC_EDIT_MSG11, m_editMsg11);
	DDX_Control(pDX, IDC_EDIT_MSG12, m_editMsg12);
	DDX_Control(pDX, IDC_BUTTON_SEND1, m_buttonSend1);
	DDX_Control(pDX, IDC_BUTTON_SEND2, m_buttonSend2);
	DDX_Control(pDX, IDC_BUTTON_SEND3, m_buttonSend3);
	DDX_Control(pDX, IDC_BUTTON_SEND4, m_buttonSend4);
	DDX_Control(pDX, IDC_BUTTON_SEND5, m_buttonSend5);
	DDX_Control(pDX, IDC_BUTTON_SEND6, m_buttonSend6);
	DDX_Control(pDX, IDC_BUTTON_SEND7, m_buttonSend7);
	DDX_Control(pDX, IDC_BUTTON_SEND8, m_buttonSend8);
	DDX_Control(pDX, IDC_BUTTON_SEND9, m_buttonSend9);
	DDX_Control(pDX, IDC_BUTTON_SEND10, m_buttonSend10);
	DDX_Control(pDX, IDC_BUTTON_SEND11, m_buttonSend11);
	DDX_Control(pDX, IDC_BUTTON_SEND12, m_buttonSend12);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputWindow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ManualDlg, ReaderControlTabDlg)
	//{{AFX_MSG_MAP(ManualDlg)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_OUTPUT, OnButtonClearOutput)
	ON_EN_SETFOCUS(IDC_EDIT_MSG1, OnSetFocusEditMsg1)
	ON_EN_SETFOCUS(IDC_EDIT_MSG2, OnSetFocusEditMsg2)
	ON_EN_SETFOCUS(IDC_EDIT_MSG3, OnSetFocusEditMsg3)
	ON_EN_SETFOCUS(IDC_EDIT_MSG4, OnSetFocusEditMsg4)
	ON_EN_SETFOCUS(IDC_EDIT_MSG5, OnSetFocusEditMsg5)
	ON_EN_SETFOCUS(IDC_EDIT_MSG6, OnSetFocusEditMsg6)
	ON_EN_SETFOCUS(IDC_EDIT_MSG7, OnSetFocusEditMsg7)
	ON_EN_SETFOCUS(IDC_EDIT_MSG8, OnSetFocusEditMsg8)
	ON_EN_SETFOCUS(IDC_EDIT_MSG9, OnSetFocusEditMsg9)
	ON_EN_SETFOCUS(IDC_EDIT_MSG10, OnSetFocusEditMsg10)
	ON_EN_SETFOCUS(IDC_EDIT_MSG11, OnSetFocusEditMsg11)
	ON_EN_SETFOCUS(IDC_EDIT_MSG12, OnSetFocusEditMsg12)
	ON_BN_CLICKED(IDC_BUTTON_SEND1, OnButtonSend1)
	ON_BN_CLICKED(IDC_BUTTON_SEND2, OnButtonSend2)
	ON_BN_CLICKED(IDC_BUTTON_SEND3, OnButtonSend3)
	ON_BN_CLICKED(IDC_BUTTON_SEND4, OnButtonSend4)
	ON_BN_CLICKED(IDC_BUTTON_SEND5, OnButtonSend5)
	ON_BN_CLICKED(IDC_BUTTON_SEND6, OnButtonSend6)
	ON_BN_CLICKED(IDC_BUTTON_SEND7, OnButtonSend7)
	ON_BN_CLICKED(IDC_BUTTON_SEND8, OnButtonSend8)
	ON_BN_CLICKED(IDC_BUTTON_SEND9, OnButtonSend9)
	ON_BN_CLICKED(IDC_BUTTON_SEND10, OnButtonSend10)
	ON_BN_CLICKED(IDC_BUTTON_SEND11, OnButtonSend11)
	ON_BN_CLICKED(IDC_BUTTON_SEND12, OnButtonSend12)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_CMDS, OnButtonClearCmds)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_COMM_INFO_RESPONSE, OnInfoResponse)
    ON_MESSAGE(WM_COMM_PENDING_RESPONSE, OnPendingResponse)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnTransactionComplete)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ManualDlg message handlers

BOOL ManualDlg::OnInitDialog() 
{
	ReaderControlTabDlg::OnInitDialog();
	
    int autoHeader = AfxGetApp()->GetProfileInt(profile_sectionManual, profile_keyAutoHeader, profile_defaultAutoHeader);

    m_editMsg1.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg1));
    m_editMsg2.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg2));
    m_editMsg3.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg3));
    m_editMsg4.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg4));
    m_editMsg5.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg5));
    m_editMsg6.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg6));
    m_editMsg7.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg7));
    m_editMsg8.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg8));
    m_editMsg9.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg9));
    m_editMsg10.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg10));
    m_editMsg11.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg11));
    m_editMsg12.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg12));

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
    m_editMsg2.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg2, msg);
    m_editMsg3.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg3, msg);
    m_editMsg4.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg4, msg);
    m_editMsg5.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg5, msg);
    m_editMsg6.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg6, msg);
    m_editMsg7.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg7, msg);
    m_editMsg8.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg8, msg);
    m_editMsg9.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg9, msg);
    m_editMsg10.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg10, msg);
    m_editMsg11.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg11, msg);
    m_editMsg12.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionManual, profile_keyMsg12, msg);

    return TRUE;
}


void ManualDlg::DisplayFPGAConnected(BOOL connected)
{
    m_outputWindow.EnableWindow(connected);
    m_editMsg1.EnableWindow(connected);
    m_editMsg2.EnableWindow(connected);
    m_editMsg3.EnableWindow(connected);
    m_editMsg4.EnableWindow(connected);
    m_editMsg5.EnableWindow(connected);
    m_editMsg6.EnableWindow(connected);
    m_editMsg7.EnableWindow(connected);
    m_editMsg8.EnableWindow(connected);
    m_editMsg9.EnableWindow(connected);
    m_editMsg10.EnableWindow(connected);
    m_editMsg11.EnableWindow(connected);
    m_editMsg12.EnableWindow(connected);
    m_buttonSend1.EnableWindow(connected);
    m_buttonSend2.EnableWindow(connected);
    m_buttonSend3.EnableWindow(connected);
    m_buttonSend4.EnableWindow(connected);
    m_buttonSend5.EnableWindow(connected);
    m_buttonSend6.EnableWindow(connected);
    m_buttonSend7.EnableWindow(connected);
    m_buttonSend8.EnableWindow(connected);
    m_buttonSend9.EnableWindow(connected);
    m_buttonSend10.EnableWindow(connected);
    m_buttonSend11.EnableWindow(connected);
    m_buttonSend12.EnableWindow(connected);

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

void ManualDlg::OnSetFocusEditMsg2() 
{
    m_activeEditBox = &m_editMsg2;
}

void ManualDlg::OnSetFocusEditMsg3() 
{
    m_activeEditBox = &m_editMsg3;
}

void ManualDlg::OnSetFocusEditMsg4() 
{
    m_activeEditBox = &m_editMsg4;
}

void ManualDlg::OnSetFocusEditMsg5() 
{
    m_activeEditBox = &m_editMsg5;
}

void ManualDlg::OnSetFocusEditMsg6() 
{
    m_activeEditBox = &m_editMsg6;
}

void ManualDlg::OnSetFocusEditMsg7() 
{
    m_activeEditBox = &m_editMsg7;
}

void ManualDlg::OnSetFocusEditMsg8() 
{
    m_activeEditBox = &m_editMsg8;
}

void ManualDlg::OnSetFocusEditMsg9() 
{
    m_activeEditBox = &m_editMsg9;
}

void ManualDlg::OnSetFocusEditMsg10() 
{
    m_activeEditBox = &m_editMsg10;
}

void ManualDlg::OnSetFocusEditMsg11() 
{
    m_activeEditBox = &m_editMsg11;
}

void ManualDlg::OnSetFocusEditMsg12() 
{
    m_activeEditBox = &m_editMsg12;
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

void ManualDlg::OnButtonSend2() 
{
    GetAndSendCmd(&m_editMsg2);
}

void ManualDlg::OnButtonSend3() 
{
    GetAndSendCmd(&m_editMsg3);
}

void ManualDlg::OnButtonSend4() 
{
    GetAndSendCmd(&m_editMsg4);
}

void ManualDlg::OnButtonSend5() 
{
    GetAndSendCmd(&m_editMsg5);
}

void ManualDlg::OnButtonSend6() 
{
    GetAndSendCmd(&m_editMsg6);
}

void ManualDlg::OnButtonSend7() 
{
    GetAndSendCmd(&m_editMsg7);
}

void ManualDlg::OnButtonSend8() 
{
    GetAndSendCmd(&m_editMsg8);
}

void ManualDlg::OnButtonSend9() 
{
    GetAndSendCmd(&m_editMsg9);
}

void ManualDlg::OnButtonSend10() 
{
    GetAndSendCmd(&m_editMsg10);
}

void ManualDlg::OnButtonSend11() 
{
    GetAndSendCmd(&m_editMsg11);
}

void ManualDlg::OnButtonSend12() 
{
    GetAndSendCmd(&m_editMsg12);
}

void ManualDlg::OnButtonClearCmds() 
{
    if(ShowMsgBox(MB_ICONQUESTION | MB_OKCANCEL, "Clearing Commands", "Clearing Manual Commands") == IDOK)
    {
        m_editMsg1.SetWindowText("");
        m_editMsg2.SetWindowText("");
        m_editMsg3.SetWindowText("");
        m_editMsg4.SetWindowText("");
        m_editMsg5.SetWindowText("");
        m_editMsg6.SetWindowText("");
        m_editMsg7.SetWindowText("");
        m_editMsg8.SetWindowText("");
        m_editMsg9.SetWindowText("");
        m_editMsg10.SetWindowText("");
        m_editMsg11.SetWindowText("");
        m_editMsg12.SetWindowText("");
    }
}

