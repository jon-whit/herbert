// ManualDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HerbertController.h"
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
const static char* profile_keyMsg2           = "msg2";


/////////////////////////////////////////////////////////////////////////////
// ManualDlg dialog


ManualDlg::ManualDlg(CWnd* parentWnd)
	: HerbertControllerTabDlg(ManualDlg::IDD, parentWnd)
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
	HerbertControllerTabDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ManualDlg)
	DDX_Control(pDX, IDC_EDIT_MSG1, m_editMsg1);
	DDX_Control(pDX, IDC_EDIT_MSG2, m_editMsg2);
	DDX_Control(pDX, IDC_BUTTON_SEND1, m_buttonSend1);
	DDX_Control(pDX, IDC_BUTTON_SEND2, m_buttonSend2);
	DDX_Control(pDX, IDC_BUTTON_LIN, m_buttonLIn);
	DDX_Control(pDX, IDC_BUTTON_LOUT, m_buttonLOut);
	DDX_Control(pDX, IDC_BUTTON_LC, m_buttonLC);
	DDX_Control(pDX, IDC_BUTTON_LCC, m_buttonLCC);
	DDX_Control(pDX, IDC_BUTTON_L2, m_buttonL2);
	DDX_Control(pDX, IDC_BUTTON_FIN, m_buttonFIn);
	DDX_Control(pDX, IDC_BUTTON_FOUT, m_buttonFOut);
	DDX_Control(pDX, IDC_BUTTON_FC, m_buttonFC);
	DDX_Control(pDX, IDC_BUTTON_FCC, m_buttonFCC);
	DDX_Control(pDX, IDC_BUTTON_F2, m_buttonF2);
	DDX_Control(pDX, IDC_BUTTON_RIN, m_buttonRIn);
	DDX_Control(pDX, IDC_BUTTON_ROUT, m_buttonROut);
	DDX_Control(pDX, IDC_BUTTON_RC, m_buttonRC);
	DDX_Control(pDX, IDC_BUTTON_RCC, m_buttonRCC);
	DDX_Control(pDX, IDC_BUTTON_R2, m_buttonR2);
	DDX_Control(pDX, IDC_BUTTON_BIN, m_buttonBIn);
	DDX_Control(pDX, IDC_BUTTON_BOUT, m_buttonBOut);
	DDX_Control(pDX, IDC_BUTTON_BC, m_buttonBC);
	DDX_Control(pDX, IDC_BUTTON_BCC, m_buttonBCC);
	DDX_Control(pDX, IDC_BUTTON_B2, m_buttonB2);
	DDX_Control(pDX, IDC_BUTTON_UIN, m_buttonUIn);
	DDX_Control(pDX, IDC_BUTTON_UOUT, m_buttonUOut);
	DDX_Control(pDX, IDC_BUTTON_UC, m_buttonUC);
	DDX_Control(pDX, IDC_BUTTON_UCC, m_buttonUCC);
	DDX_Control(pDX, IDC_BUTTON_U2, m_buttonU2);
	DDX_Control(pDX, IDC_BUTTON_DIN, m_buttonDIn);
	DDX_Control(pDX, IDC_BUTTON_DOUT, m_buttonDOut);
	DDX_Control(pDX, IDC_BUTTON_DC, m_buttonDC);
	DDX_Control(pDX, IDC_BUTTON_DCC, m_buttonDCC);
	DDX_Control(pDX, IDC_BUTTON_D2, m_buttonD2);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputWindow);

	DDX_Control(pDX, IDC_BUTTON_UPGRADE_FPGA, m_buttonUpgradeFpga);
	DDX_Control(pDX, IDC_BUTTON_UPGRADE_FIRMWARE, m_buttonUpgradeFirmware);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_CAPTURE, m_buttonCaptureDebugOutput);
	DDX_Control(pDX, IDC_BUTTON_CLEAR_OUTPUT, m_buttonClearOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ManualDlg, HerbertControllerTabDlg)
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
	ON_EN_SETFOCUS(IDC_EDIT_MSG2, &ManualDlg::OnEnSetfocusEditMsg2)
	ON_BN_CLICKED(IDC_BUTTON_SEND2, &ManualDlg::OnBnClickedButtonSend2)
	ON_BN_CLICKED(IDC_BUTTON_LIN, &ManualDlg::OnBnClickedButtonLin)
	ON_BN_CLICKED(IDC_BUTTON_LOUT, &ManualDlg::OnBnClickedButtonLout)
	ON_BN_CLICKED(IDC_BUTTON_LC, &ManualDlg::OnBnClickedButtonLc)
	ON_BN_CLICKED(IDC_BUTTON_LCC, &ManualDlg::OnBnClickedButtonLcc)
	ON_BN_CLICKED(IDC_BUTTON_L2, &ManualDlg::OnBnClickedButtonL2)
	ON_BN_CLICKED(IDC_BUTTON_FIN, &ManualDlg::OnBnClickedButtonFin)
	ON_BN_CLICKED(IDC_BUTTON_FOUT, &ManualDlg::OnBnClickedButtonFout)
	ON_BN_CLICKED(IDC_BUTTON_FC, &ManualDlg::OnBnClickedButtonFc)
	ON_BN_CLICKED(IDC_BUTTON_FCC, &ManualDlg::OnBnClickedButtonFcc)
	ON_BN_CLICKED(IDC_BUTTON_F2, &ManualDlg::OnBnClickedButtonF2)
	ON_BN_CLICKED(IDC_BUTTON_RIN, &ManualDlg::OnBnClickedButtonRin)
	ON_BN_CLICKED(IDC_BUTTON_ROUT, &ManualDlg::OnBnClickedButtonRout)
	ON_BN_CLICKED(IDC_BUTTON_RC, &ManualDlg::OnBnClickedButtonRc)
	ON_BN_CLICKED(IDC_BUTTON_RCC, &ManualDlg::OnBnClickedButtonRcc)
	ON_BN_CLICKED(IDC_BUTTON_R2, &ManualDlg::OnBnClickedButtonR2)
	ON_BN_CLICKED(IDC_BUTTON_BIN, &ManualDlg::OnBnClickedButtonBin)
	ON_BN_CLICKED(IDC_BUTTON_BOUT, &ManualDlg::OnBnClickedButtonBout)
	ON_BN_CLICKED(IDC_BUTTON_BC, &ManualDlg::OnBnClickedButtonBc)
	ON_BN_CLICKED(IDC_BUTTON_BCC, &ManualDlg::OnBnClickedButtonBcc)
	ON_BN_CLICKED(IDC_BUTTON_B2, &ManualDlg::OnBnClickedButtonB2)
	ON_BN_CLICKED(IDC_BUTTON_UIN, &ManualDlg::OnBnClickedButtonUin)
	ON_BN_CLICKED(IDC_BUTTON_UOUT, &ManualDlg::OnBnClickedButtonUout)
	ON_BN_CLICKED(IDC_BUTTON_UC, &ManualDlg::OnBnClickedButtonUc)
	ON_BN_CLICKED(IDC_BUTTON_UCC, &ManualDlg::OnBnClickedButtonUcc)
	ON_BN_CLICKED(IDC_BUTTON_U2, &ManualDlg::OnBnClickedButtonU2)
	ON_BN_CLICKED(IDC_BUTTON_DIN, &ManualDlg::OnBnClickedButtonDin)
	ON_BN_CLICKED(IDC_BUTTON_DOUT, &ManualDlg::OnBnClickedButtonDout)
	ON_BN_CLICKED(IDC_BUTTON_DC, &ManualDlg::OnBnClickedButtonDc)
	ON_BN_CLICKED(IDC_BUTTON_DCC, &ManualDlg::OnBnClickedButtonDcc)
	ON_BN_CLICKED(IDC_BUTTON_D2, &ManualDlg::OnBnClickedButtonD2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ManualDlg message handlers

BOOL ManualDlg::OnInitDialog() 
{
	HerbertControllerTabDlg::OnInitDialog();
	
    int autoHeader = AfxGetApp()->GetProfileInt(profile_sectionManual, profile_keyAutoHeader, profile_defaultAutoHeader);

    m_editMsg1.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg1));
	m_editMsg2.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionManual, profile_keyMsg2));

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

    return TRUE;
}


void ManualDlg::DisplayFPGAConnected(BOOL connected)
{
    m_outputWindow.EnableWindow(connected);
    m_editMsg1.EnableWindow(connected);
	m_editMsg2.EnableWindow(connected);
    m_buttonSend1.EnableWindow(connected);
	m_buttonSend2.EnableWindow(connected);

	m_buttonClearOutput.EnableWindow(connected);
	m_buttonCaptureDebugOutput.EnableWindow(connected);
	m_buttonUpgradeFirmware.EnableWindow(connected);
	m_buttonUpgradeFpga.EnableWindow(connected);

	m_buttonLIn.EnableWindow(connected);
	m_buttonLOut.EnableWindow(connected);
	m_buttonLC.EnableWindow(connected);
	m_buttonLCC.EnableWindow(connected);
	m_buttonL2.EnableWindow(connected);
	m_buttonFIn.EnableWindow(connected);
	m_buttonFOut.EnableWindow(connected);
	m_buttonFC.EnableWindow(connected);
	m_buttonFCC.EnableWindow(connected);
	m_buttonF2.EnableWindow(connected);
	m_buttonRIn.EnableWindow(connected);
	m_buttonROut.EnableWindow(connected);
	m_buttonRC.EnableWindow(connected);
	m_buttonRCC.EnableWindow(connected);
	m_buttonR2.EnableWindow(connected);
	m_buttonBIn.EnableWindow(connected);
	m_buttonBOut.EnableWindow(connected);
	m_buttonBC.EnableWindow(connected);
	m_buttonBCC.EnableWindow(connected);
	m_buttonB2.EnableWindow(connected);
	m_buttonUIn.EnableWindow(connected);
	m_buttonUOut.EnableWindow(connected);
	m_buttonUC.EnableWindow(connected);
	m_buttonUCC.EnableWindow(connected);
	m_buttonU2.EnableWindow(connected);
	m_buttonDIn.EnableWindow(connected);
	m_buttonDOut.EnableWindow(connected);
	m_buttonDC.EnableWindow(connected);
	m_buttonDCC.EnableWindow(connected);
	m_buttonD2.EnableWindow(connected);


    m_connected = connected;
}



void ManualDlg::OnTimer(UINT nIDEvent) 
{
    KillTimer(nIDEvent);
	
	HerbertControllerTabDlg::OnTimer(nIDEvent);
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

void ManualDlg::OnEnSetfocusEditMsg2()
{
	 m_activeEditBox = &m_editMsg2;
}


void ManualDlg::OnBnClickedButtonSend2()
{
	GetAndSendCmd(&m_editMsg2);
}


void ManualDlg::OnBnClickedButtonLin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("L"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonLout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonLc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"L", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonLcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"L", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonL2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"L", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonFin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("F"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonFout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonFc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"F", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonFcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"F", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonF2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"F", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonRin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("R"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonRout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonRc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"R", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonRcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"R", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonR2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"R", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonBin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("B"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonBout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonBc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"B", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonBcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"B", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonB2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"B", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonUin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("U"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonUout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonUc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"U", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonUcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"U", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonU2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"U", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonDin()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("D"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonDout()
{
	Commands::Transaction::shared_ptr cmd(new Commands::ActuateArm("O"));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonDc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"D", -50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonDcc()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"D", 50));
    m_comm.sendCommand(m_rspQueue, cmd);
}


void ManualDlg::OnBnClickedButtonD2()
{
	Commands::Transaction::shared_ptr cmd(new Commands::MoveRelative((std::string)"D", 100));
    m_comm.sendCommand(m_rspQueue, cmd);
}
