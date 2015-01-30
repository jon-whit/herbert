// PcbDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PcbDlg.h"
#include <Messages.h>



#define NORMAL_TEXT_COLOR                  RGB(0,0,0)
#define NORMAL_BACKGROUND_COLOR            RGB(255,255,255)

#define CHANGING_TEXT_COLOR                RGB(0,150,0)
#define CHANGING_BACKGROUND_COLOR          RGB(240,240,240)

#define ERROR_CHANGING_TEXT_COLOR          RGB(0,0,0)
#define ERROR_CHANGING_BACKGROUND_COLOR    RGB(255,0,0)

#define ERROR_TEXT_COLOR                   RGB(255,0,0)
#define ERROR_BACKGROUND_COLOR             RGB(255,200,200)

#define DISABLED_TEXT_COLOR                RGB(150,150,150)
#define DISABLED_BACKGROUND_COLOR          RGB(255,255,255)

#define TEXT_COLOR_OFF                     RGB(255,0,0)
#define TEXT_COLOR_ON                      RGB(0,255,0)

#define TEXT_COLOR_STATE_REQ_PWR_DOWN      RGB(255,0,0)
#define TEXT_COLOR_STATE_REQ_NONE          RGB(0,0,0)


#define POLL_INFO_TIMER         1
#define POLL_STATUS_TIMER       2

#define POLL_INFO_PERIOD_ms     5000
#define POLL_STATUS_PERIOD_ms   2000


enum StatusWindowColumnTypes
{
    COLUMN_0_UNUSED  = 0,
    COLUMN_COMPONENT = 1,
    COLUMN_POWER     = 2,
    COLUMN_VOLTAGE   = 3,
    COLUMN_ERROR     = 4,
};


struct
{
    const double widthPercent;
    TCHAR* title;
    const int    mask;
    const int    format;
} statusWindowColumns[] =
{
    { 0, "",           LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_CENTER }, //LVCFMT_LEFT   },
    { 37, "Component", LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_CENTER }, //LVCFMT_LEFT   },
    { 12, "Power",     LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_CENTER },
    { 16, "Voltage",   LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_CENTER },
    { 34, "Error",     LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_CENTER },

    // End of table marker
    { 0, 0, 0, 0 },
};

enum StatusWindowComponents
{
    COMPONENT_ATX_SUPPLY,
    COMPONENT_POWER3_3V,
    COMPONENT_POWER12V,
    COMPONENT_POWER24V,
    COMPONENT_LED_CTRL,
    COMPONENT_SENSOR,
    COMPONENT_LID,
    COMPONENT_FANS,
};

struct
{
    const char* component;
    const bool  powerAvailable;
    const bool  voltageAvailable;
    const bool  errorAvailable;
} statusWindowRows[] =
{  // Name                        Power  Voltage  Error
    { "ATX Power Supply",         true,  false, false },
    { "Power Supply - 3.3v",      false, true,  true  },
    { "Power Supply - 12v",       false, true,  true  },
    { "Power Supply - 24v",       true,  true,  true  },
    { "Excitation LEDs",          true,  false, false },
    { "Sensor Interface",         true,  true,  true  },
    { "Heated Lid",               true,  false, false },
    { "Fans",                     true,  false, false },

    // End of table marker
    { NULL, false, false, false }
};

// PcbDlg dialog


const static char profile_sectionPrbCtrl[]  = "PCB Control Settings";

const static char profile_keyCommPort[]     = "COM Port";
const static int  profile_defaultCommPort   = 1;
const static int  profile_maxCommPort       = 19;
const static char profile_keyPollStatus[]   = "Poll Status";

const static char profile_keyMsg1[]         = "msg1";
const static char profile_keyMsg2[]         = "msg2";
const static char profile_keyMsg3[]         = "msg3";
const static char profile_keyMsg4[]         = "msg4";



IMPLEMENT_DYNAMIC(PcbDlg, CDialog)

PcbDlg::PcbDlg(CWnd* pParent)
    : ReaderControlTabDlg(PcbDlg::IDD, pParent)
    , m_pcbComm(PcbComm::getInstance())
    , m_manualRspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
    , m_cmdRspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE + 1))
{

}

PcbDlg::~PcbDlg()
{
}

void PcbDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_MSG1, m_editMsg1);
    DDX_Control(pDX, IDC_EDIT_MSG2, m_editMsg2);
    DDX_Control(pDX, IDC_EDIT_MSG3, m_editMsg3);
    DDX_Control(pDX, IDC_EDIT_MSG4, m_editMsg4);
    DDX_Control(pDX, IDC_BUTTON_SEND1, m_buttonSend1);
    DDX_Control(pDX, IDC_BUTTON_SEND2, m_buttonSend2);
    DDX_Control(pDX, IDC_BUTTON_SEND3, m_buttonSend3);
    DDX_Control(pDX, IDC_BUTTON_SEND4, m_buttonSend4);
    DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editOutputWindow);
    DDX_Control(pDX, IDC_LIST_STATUS, m_listStatus);
    DDX_Control(pDX, IDC_CHECK_POLL_STATUS, m_checkPollSystemStatus);
    DDX_Control(pDX, IDC_STATIC_PWR_DOWN_REQUESTED, m_staticPwrDownRequested);
}


BEGIN_MESSAGE_MAP(PcbDlg, ReaderControlTabDlg)
    ON_BN_CLICKED(IDC_BUTTON_SEND1, &PcbDlg::OnBnClickedButtonSend1)
    ON_BN_CLICKED(IDC_BUTTON_SEND2, &PcbDlg::OnBnClickedButtonSend2)
    ON_BN_CLICKED(IDC_BUTTON_SEND3, &PcbDlg::OnBnClickedButtonSend3)
    ON_BN_CLICKED(IDC_BUTTON_SEND4, &PcbDlg::OnBnClickedButtonSend4)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_OUTPUT, &PcbDlg::OnBnClickedButtonClearOutput)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_CMDS, &PcbDlg::OnBnClickedButtonClearCmds)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnManualTransactionComplete)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE + 1, OnCmdTransactionComplete)
    ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_CHECKBOX_CLICKED, OnCheckbox)
    ON_BN_CLICKED(IDC_CHECK_POLL_STATUS, &PcbDlg::OnBnClickedCheckPollStatus)
END_MESSAGE_MAP()


// PcbDlg message handlers
BOOL PcbDlg::OnInitDialog()
{
    ReaderControlTabDlg::OnInitDialog();



    ////////////////////////////////////
    // Protocol List Control

    CRect rect;
    m_listStatus.GetWindowRect(&rect);

    int statusWindowWidth  = rect.Width() - 2;
    int total_cx           = 0;

    // add columns
    for (int i = 0; statusWindowColumns[i].title != NULL; i++)
    {
        LV_COLUMN lvcolumn;
        memset(&lvcolumn, 0, sizeof(lvcolumn));

        lvcolumn.mask     = statusWindowColumns[i].mask;
        lvcolumn.fmt      = statusWindowColumns[i].format;
        lvcolumn.pszText  = statusWindowColumns[i].title;
        lvcolumn.iSubItem = i;
        lvcolumn.cx       = (int)(statusWindowColumns[i].widthPercent * statusWindowWidth / 100);

        total_cx += lvcolumn.cx;
        m_listStatus.InsertColumn(i, &lvcolumn);
    }

	// XListCtrl must have LVS_EX_FULLROWSELECT if combo boxes are used
	m_listStatus.SetExtendedStyle(LVS_EX_FULLROWSELECT);// | LVS_EX_TRACKSELECT);

	//m_listStatus.EnableToolTips(TRUE);



    m_listStatus.LockWindowUpdate();

    for (int row = 0; statusWindowRows[row].component != NULL; row++)
    {
        m_listStatus.InsertItem(row, "");
    }

    m_listStatus.UnlockWindowUpdate();



    m_checkPollSystemStatus.SetCheck(!!AfxGetApp()->GetProfileInt(profile_sectionPrbCtrl, profile_keyPollStatus, 0));



    m_editMsg1.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionPrbCtrl, profile_keyMsg1));
    m_editMsg2.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionPrbCtrl, profile_keyMsg2));
    m_editMsg3.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionPrbCtrl, profile_keyMsg3));
    m_editMsg4.SetWindowText(AfxGetApp()->GetProfileString(profile_sectionPrbCtrl, profile_keyMsg4));

    SetUpSystemStatus();

    DisplayPCBConnected(false);

    m_staticPwrDownRequested.SetWindowText("None");
    m_staticPwrDownRequested.SetTextColor(TEXT_COLOR_STATE_REQ_NONE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}



void PcbDlg::OnOK()
{
    CWnd* focusedWindow = GetFocus();

    if(focusedWindow == &m_editMsg1 ||
       focusedWindow == &m_editMsg2 ||
       focusedWindow == &m_editMsg3 ||
       focusedWindow == &m_editMsg4)
    {
        GetAndSendManualCmd((CEdit*)focusedWindow);
    }
}



void PcbDlg::OnAbort()
{
}


BOOL PcbDlg::SaveSettingsAndExit()
{
    m_pcbComm.Close();

    AfxGetApp()->WriteProfileInt(profile_sectionPrbCtrl, profile_keyPollStatus, m_checkPollSystemStatus.GetCheck());

    CString msg;
    m_editMsg1.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionPrbCtrl, profile_keyMsg1, msg);
    m_editMsg2.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionPrbCtrl, profile_keyMsg2, msg);
    m_editMsg3.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionPrbCtrl, profile_keyMsg3, msg);
    m_editMsg4.GetWindowText(msg); AfxGetApp()->WriteProfileString(profile_sectionPrbCtrl, profile_keyMsg4, msg);
    return TRUE;
}


void PcbDlg::DisplayFPGAConnected(BOOL connected)
{
}


void PcbDlg::DisplayPCBConnected(BOOL connected)
{
    if(connected)
    {
        SetUpSystemStatus();
    }
    m_editOutputWindow.EnableWindow(connected);
    m_editMsg1.EnableWindow(connected);
    m_editMsg2.EnableWindow(connected);
    m_editMsg3.EnableWindow(connected);
    m_editMsg4.EnableWindow(connected);
    m_buttonSend1.EnableWindow(connected);
    m_buttonSend2.EnableWindow(connected);
    m_buttonSend3.EnableWindow(connected);
    m_buttonSend4.EnableWindow(connected);
    m_listStatus.EnableWindow(connected);
}



void PcbDlg::SetUpSystemStatus()
{
    m_listStatus.LockWindowUpdate();
    for(int row = 0; statusWindowRows[row].component != NULL; row++)
    {
        m_listStatus.SetItemText(row, COLUMN_COMPONENT, statusWindowRows[row].component, NORMAL_TEXT_COLOR, NORMAL_BACKGROUND_COLOR);
        m_listStatus.SetBold(row, COLUMN_COMPONENT, true);

        if(!statusWindowRows[row].powerAvailable)
        {
            m_listStatus.SetItemText(row, COLUMN_POWER, "N/A", DISABLED_TEXT_COLOR, DISABLED_BACKGROUND_COLOR);
        }
        else
        {
            m_listStatus.SetItemText(row, COLUMN_POWER, "Off", TEXT_COLOR_OFF, NORMAL_BACKGROUND_COLOR);
            m_listStatus.SetCheckbox(row, COLUMN_POWER, 0);
        }

        if(!statusWindowRows[row].voltageAvailable)
        {
            m_listStatus.SetItemText(row, COLUMN_VOLTAGE, "N/A", DISABLED_TEXT_COLOR, DISABLED_BACKGROUND_COLOR);
        }
        else
        {
            m_listStatus.SetItemText(row, COLUMN_VOLTAGE, "", NORMAL_TEXT_COLOR, NORMAL_BACKGROUND_COLOR);
        }

        if(!statusWindowRows[row].errorAvailable)
        {
            m_listStatus.SetItemText(row, COLUMN_ERROR, "N/A", DISABLED_TEXT_COLOR, DISABLED_BACKGROUND_COLOR);
        }
        else
        {
            m_listStatus.SetItemText(row, COLUMN_ERROR, "", NORMAL_TEXT_COLOR, NORMAL_BACKGROUND_COLOR);
        }
    }
    m_listStatus.UnlockWindowUpdate();

    OnBnClickedCheckPollStatus();
}



void PcbDlg::GetSystemStatus()
{
    Commands::Transaction::shared_ptr cmd;
        
    cmd.reset(new Commands::Get3_3VSupplyVoltage());
    m_pendingStatusCommands.push_back(cmd);

    cmd.reset(new Commands::GetPcbStatus());
    m_pendingStatusCommands.push_back(cmd);

    cmd.reset(new Commands::Get12VSupplyVoltage());
    m_pendingStatusCommands.push_back(cmd);

    cmd.reset(new Commands::Get24VSupplyVoltage());
    m_pendingStatusCommands.push_back(cmd);

    cmd.reset(new Commands::GetSensorVoltage());
    m_pendingStatusCommands.push_back(cmd);


    GetOnStates();


    SendNextCmd();
}


void PcbDlg::GetOnStates()
{
    Commands::Transaction::shared_ptr cmd;

    cmd.reset(new Commands::GetATXSupplyOnState());
    m_pendingInfoCommands.push_back(cmd);

    cmd.reset(new Commands::Get24VSupplyOnState());
    m_pendingInfoCommands.push_back(cmd);

    cmd.reset(new Commands::GetLEDCtrlOnState());
    m_pendingInfoCommands.push_back(cmd);

    cmd.reset(new Commands::GetSensorOnState());
    m_pendingInfoCommands.push_back(cmd);

    cmd.reset(new Commands::GetLidOnState());
    m_pendingInfoCommands.push_back(cmd);

    cmd.reset(new Commands::GetFansOnState());
    m_pendingInfoCommands.push_back(cmd);
}



void PcbDlg::OnBnClickedCheckPollStatus()
{
    if(m_checkPollSystemStatus.GetCheck())
    {
        GetSystemStatus();
        SetTimer(POLL_STATUS_TIMER, POLL_STATUS_PERIOD_ms, NULL);
    }
    else
    {
        KillTimer(POLL_STATUS_TIMER);
    }
}



LRESULT PcbDlg::OnCheckbox(WPARAM nItem, LPARAM nSubItem)
{
    int state = m_listStatus.GetCheckbox(nItem, nSubItem);

    Commands::Transaction::shared_ptr cmd;

    switch(nItem)
    {
    case COMPONENT_ATX_SUPPLY:
        cmd.reset(new Commands::SetATXSupplyOnState(state));
        break;
    case COMPONENT_POWER24V:
        cmd.reset(new Commands::Set24VSupplyOnState(state));
        break;
    case COMPONENT_LED_CTRL:
        cmd.reset(new Commands::SetLEDCtrlOnState(state));
        break;
    case COMPONENT_SENSOR:
        cmd.reset(new Commands::SetSensorOnState(state));
        break;
    case COMPONENT_LID:
        cmd.reset(new Commands::SetLidOnState(state));
        break;
    case COMPONENT_FANS:
        cmd.reset(new Commands::SetFansOnState(state));
        break;
    default:
        //TODO: handle error
        break;
    }

    m_pendingInfoCommands.push_back(cmd);
    GetOnStates();
    SendNextCmd();

    return 0;
}


LRESULT PcbDlg::OnManualTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_manualRspQueue->dequeue());

    if(transaction)
    {
        if(transaction->validRsp() || transaction->status().GetLength())
        {
            CString rspPktString = transaction->rspPktString();

            m_editOutputWindow.SetSel(0, -1);
            m_editOutputWindow.SetSel(-1, -1);
            m_editOutputWindow.ReplaceSel(rspPktString + "\r\n");
        }
        else
        {
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "'%s'", transaction->getErrorString());
        }
    }

    return 0;
}



LRESULT PcbDlg::OnCmdTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_cmdRspQueue->dequeue());

    if(transaction)
    {
        if(transaction->validRsp() && transaction->statusOk())
        {
            // Handle successful response
            switch(transaction->type())
            {
                // Status Responsese
                case Commands::Transaction::TypeGetPcbStatus:
                {
                    Commands::GetPcbStatus* rsp = dynamic_cast<Commands::GetPcbStatus*>(transaction.get());

                    UpdateErrorDisplay(COMPONENT_POWER3_3V,   rsp->PowerSupply3_3VError(), rsp->PowerSupply3_3VVoltage());
                    UpdateErrorDisplay(COMPONENT_POWER12V,    rsp->PowerSupply12VError(),  rsp->PowerSupply12VVoltage());
                    UpdateErrorDisplay(COMPONENT_POWER24V,    rsp->PowerSupply24VError(),  rsp->PowerSupply24VVoltage());
                    UpdateErrorDisplay(COMPONENT_SENSOR,      rsp->SensorPowerError(),     rsp->SensorPowerVoltage());

                    if(!rsp->PwrDownRequested())
                    {
                        m_staticPwrDownRequested.SetWindowText("None");
                        m_staticPwrDownRequested.SetTextColor(TEXT_COLOR_STATE_REQ_NONE);
                    }
                    else
                    {
                        m_staticPwrDownRequested.SetWindowText("Pwr Down");
                        m_staticPwrDownRequested.SetTextColor(TEXT_COLOR_STATE_REQ_PWR_DOWN);
                    }

                    break;
                }

                case Commands::Transaction::TypeGet3_3VSupplyVoltage:
                {
                    Commands::Get3_3VSupplyVoltage* rsp = dynamic_cast<Commands::Get3_3VSupplyVoltage*>(transaction.get());
                    UpdateVoltageDisplay(COMPONENT_POWER3_3V, rsp->Voltage());
                    break;
                }

                case Commands::Transaction::TypeGet12VSupplyVoltage:
                {
                    Commands::Get12VSupplyVoltage* rsp = dynamic_cast<Commands::Get12VSupplyVoltage*>(transaction.get());
                    UpdateVoltageDisplay(COMPONENT_POWER12V, rsp->Voltage());
                    break;
                }

                case Commands::Transaction::TypeGet24VSupplyVoltage:
                {
                    Commands::Get24VSupplyVoltage* rsp = dynamic_cast<Commands::Get24VSupplyVoltage*>(transaction.get());
                    UpdateVoltageDisplay(COMPONENT_POWER24V, rsp->Voltage());
                    break;
                }

                case Commands::Transaction::TypeGetSensorVoltage:
                {
                    Commands::GetSensorVoltage* rsp = dynamic_cast<Commands::GetSensorVoltage*>(transaction.get());
                    UpdateVoltageDisplay(COMPONENT_SENSOR, rsp->Voltage());
                    break;
                }

                case Commands::Transaction::TypeGetATXSupplyOnState:
                {
                    Commands::GetATXSupplyOnState* rsp = dynamic_cast<Commands::GetATXSupplyOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_ATX_SUPPLY, rsp->On());
                    break;
                }

                case Commands::Transaction::TypeGet24VSupplyOnState:
                {
                    Commands::Get24VSupplyOnState* rsp = dynamic_cast<Commands::Get24VSupplyOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_POWER24V, rsp->On());
                    break;
                }

                case Commands::Transaction::TypeGetLEDCtrlOnState:
                {
                    Commands::GetLEDCtrlOnState* rsp = dynamic_cast<Commands::GetLEDCtrlOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_LED_CTRL, rsp->On());
                    break;
                }

                case Commands::Transaction::TypeGetSensorOnState:
                {
                    Commands::GetSensorOnState* rsp = dynamic_cast<Commands::GetSensorOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_SENSOR, rsp->On());
                    break;
                }

                case Commands::Transaction::TypeGetLidOnState:
                {
                    Commands::GetLidOnState* rsp = dynamic_cast<Commands::GetLidOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_LID, rsp->On());
                    break;
                }

                case Commands::Transaction::TypeGetFansOnState:
                {
                    Commands::GetFansOnState* rsp = dynamic_cast<Commands::GetFansOnState*>(transaction.get());
                    UpdateOnStateDisplay(COMPONENT_FANS, rsp->On());
                    break;
                }

                default:
                    // Do nothing
                    break;
            }
        }
        else
        {
            // Handle failed transaction
        }
    }

    SendNextCmd();

    return 0;
}


void PcbDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(m_pcbComm.IsOpen())
    {
        switch(nIDEvent)
        {
        case POLL_INFO_TIMER:
            {
                if(m_pendingInfoCommands.empty())
                {
                    //GetSystemInfo();
                }
            }
            break;

        case POLL_STATUS_TIMER:
            {
                if(m_pendingStatusCommands.empty())
                {
                    GetSystemStatus();
                }
            }
            break;

        default:
            KillTimer(nIDEvent);
            break;
        }
    }
    else
    {
        KillTimer(nIDEvent);
    }


    ReaderControlTabDlg::OnTimer(nIDEvent);
}

void PcbDlg::GetAndSendManualCmd(CEdit* editMsg)
{
    CString cmd;
    editMsg->GetWindowText(cmd);
    if(cmd.GetLength()) SendManualCmd(cmd);
}


void PcbDlg::SendManualCmd(CString& cmd)
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

        Commands::Transaction::shared_ptr command(new Commands::PcbManual(cmd, true));
        m_pcbComm.sendCommand(m_manualRspQueue, command);
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



void PcbDlg::SendCmd(Commands::Transaction::shared_ptr command)
{
    try
    {
        if(m_pcbComm.IsOpen())
        {
            m_pcbComm.sendCommand(m_cmdRspQueue, command);
        }
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


void PcbDlg::SendNextCmd()
{
    if(!m_pendingInfoCommands.empty())
    {
        Commands::Transaction::shared_ptr cmd = m_pendingInfoCommands.front();
        m_pendingInfoCommands.pop_front();

        SendCmd(cmd);
    }
    else if(!m_pendingStatusCommands.empty())
    {
        Commands::Transaction::shared_ptr cmd = m_pendingStatusCommands.front();
        m_pendingStatusCommands.pop_front();

        SendCmd(cmd);
    }
}


void PcbDlg::OnBnClickedButtonSend1()
{
    GetAndSendManualCmd(&m_editMsg1);
}

void PcbDlg::OnBnClickedButtonSend2()
{
    GetAndSendManualCmd(&m_editMsg2);
}

void PcbDlg::OnBnClickedButtonSend3()
{
    GetAndSendManualCmd(&m_editMsg3);
}

void PcbDlg::OnBnClickedButtonSend4()
{
    GetAndSendManualCmd(&m_editMsg4);
}

void PcbDlg::OnBnClickedButtonClearOutput()
{
    m_editOutputWindow.SetWindowText("");
}

void PcbDlg::OnBnClickedButtonClearCmds()
{
    if(ShowMsgBox(MB_ICONQUESTION | MB_OKCANCEL, "Clearing Commands", "Clearing Manual Commands") == IDOK)
    {
        m_editMsg1.SetWindowText("");
        m_editMsg2.SetWindowText("");
        m_editMsg3.SetWindowText("");
        m_editMsg4.SetWindowText("");
    }
}



void PcbDlg::UpdateOnStateDisplay(int row, BOOL on)
{
    if(on)
    {
        m_listStatus.SetItemText(row, COLUMN_POWER, "On", TEXT_COLOR_OFF, NORMAL_BACKGROUND_COLOR);
        m_listStatus.SetCheckbox(row, COLUMN_POWER, 1);
    }
    else
    {
        m_listStatus.SetItemText(row, COLUMN_POWER, "Off", TEXT_COLOR_OFF, NORMAL_BACKGROUND_COLOR);
        m_listStatus.SetCheckbox(row, COLUMN_POWER, 0);
    }
}



void PcbDlg::UpdateVoltageDisplay(int row, const CString& value)
{
    CString voltageString = value + "V";
    COLORREF textColor;
    COLORREF backgroundColor;

    if(m_listStatus.GetItemText(row, COLUMN_VOLTAGE) == voltageString)
    {
        textColor       = NORMAL_TEXT_COLOR;
        backgroundColor = NORMAL_BACKGROUND_COLOR;
    }
    else
    {
        textColor       = CHANGING_TEXT_COLOR;
        backgroundColor = CHANGING_BACKGROUND_COLOR;
    }
    m_listStatus.SetItemText(row, COLUMN_VOLTAGE, voltageString, textColor, backgroundColor);
}


void PcbDlg::UpdateErrorDisplay(int row, BOOL error, const CString& value)
{
    CString errorString = "Voltage Error";
    
    if(value.GetLength())
    {
        errorString += ": " + value + "V";
    }

    if(error)
    {
        if(m_listStatus.GetItemText(row, COLUMN_ERROR).IsEmpty())
        {
            m_listStatus.SetItemText(row, COLUMN_ERROR, errorString, ERROR_CHANGING_TEXT_COLOR, ERROR_CHANGING_BACKGROUND_COLOR);
        }
        else
        {
            m_listStatus.SetItemText(row, COLUMN_ERROR, errorString, ERROR_TEXT_COLOR, ERROR_BACKGROUND_COLOR);
        }
    }
    else
    {
        m_listStatus.SetItemText(row, COLUMN_ERROR, "", NORMAL_TEXT_COLOR, NORMAL_BACKGROUND_COLOR);
    }
}

