// HerbertControllerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HerbertController.h"
#include "HerbertControllerDlg.h"
#include <Messages.h>
#include <Instrument.h>
#include <Version.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const static char profile_sectionApp[]          = "App Settings";
const static char profile_keyTabSelection[]     = "Tab Selection";
static const char profile_keyDialogXPos[]       = "Dialog X Position";
static const char profile_keyDialogYPos[]       = "Dialog Y Position";

const static char profile_sectionComm[]         = "COM Settings";
const static char profile_keyFPGAPort[]         = "FPGA Port";
const static char profile_defaultFPGACommPort[] = "COM1";


/////////////////////////////////////////////////////////////////////////////
// CHerbertControllerDlg dialog

CHerbertControllerDlg::CHerbertControllerDlg(CWnd* pParent /*=NULL*/)
    : DialogEx(CHerbertControllerDlg::IDD, pParent)
    , m_rspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
    , m_fpgaComm(FpgaComm::getInstance())
    , m_tabDlgManual(this)
    , m_waitingForResponseCount(0)
    , m_instrument(Instrument::getInstance())
    , m_instrumentEventQueue(new GuiInstrumentEventQueue(this, WM_INSTRUMENT_EVENT))
{
    //{{AFX_DATA_INIT(CHerbertControllerDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_RC_LARGE);
}

CHerbertControllerDlg::~CHerbertControllerDlg()
{
    m_fpgaComm.Close();
}

void CHerbertControllerDlg::DoDataExchange(CDataExchange* pDX)
{
    DialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_buttonStop);
    DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
    DDX_Control(pDX, IDC_COMBO_FPGACOMMPORT, m_comboFPGACommPort);
    DDX_Control(pDX, IDC_TABS, m_tabs);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_VER_STR, m_staticFPGAFirmwareVersion);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_BUILD_DATE, m_staticFPGAFirmwareBuildDate);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_VERSION_LABEL, m_staticFPGAFirmwareVersionLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_BUILD_DATE_LABEL, m_staticFPGAFirmwareBuildDateLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_VERSION_LABEL, m_staticFPGAVersionLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_VER_STR, m_staticFPGAVersion);

}

BEGIN_MESSAGE_MAP(CHerbertControllerDlg, DialogEx)
    //{{AFX_MSG_MAP(CHerbertControllerDlg)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
    ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
    //}}AFX_MSG_MAP

    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnTransactionComplete)
    ON_MESSAGE(WM_INSTRUMENT_EVENT, OnInstrumentEvent)
    ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHerbertControllerDlg message handlers

BOOL CHerbertControllerDlg::OnInitDialog()
{
    DialogEx::OnInitDialog();

        CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    CString windowTitle;
    
    GetWindowText(windowTitle);

    windowTitle = PROJECTNAME " " + windowTitle + " - Version: " APP_VERSION_NUMBER_STR;

    SetWindowText(windowTitle);


    // Configure Stop button
    COLORREF transparentColor = RGB(0, 0, 0); //(192, 192, 192);
    m_buttonStop.SetBitmaps(IDB_BITMAP_STOP, transparentColor);
    m_buttonStop.SetWindowText("");


    // Configure Info Display    
    m_staticFPGAFirmwareVersion.SetWindowText("...");
    m_staticFPGAFirmwareBuildDate.SetWindowText("...");
    m_staticFPGAVersion.SetWindowText("...");

    // Set Dialog position
    int xPos = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyDialogXPos, -1);
    int yPos = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyDialogYPos, -1);

    SetWindowPos(NULL, xPos, yPos, 0, 0, SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOSIZE);
    

    // Configure Tab Dialogs
    m_tabs.addDialog(m_tabDlgManual, "Manual Control");

    m_tabDialogs.Add(&m_tabDlgManual);
    
    int currentTab = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyTabSelection, 0);
    m_tabs.changeTab((currentTab >= 0 && currentTab < m_tabs.GetItemCount()) ? currentTab : 0);


    // Configure Com port
    CString fpgaPort = AfxGetApp()->GetProfileString(profile_sectionComm, profile_keyFPGAPort, profile_defaultFPGACommPort);

    UpdateCommPorts(fpgaPort);

    m_instrument.watchErrorMessages(m_instrumentEventQueue);
    m_instrument.watchExceptions(m_instrumentEventQueue);
    m_instrument.watchFPGAInfoUpdate(m_instrumentEventQueue);
    
    DisplayConnected();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHerbertControllerDlg::UpdateCommPorts()
{
    CString fpgaPort;
    m_comboFPGACommPort.GetWindowText(fpgaPort);
    UpdateCommPorts(fpgaPort);
}


void CHerbertControllerDlg::UpdateCommPorts(const CString& fpgaPort)
{
    while(m_comboFPGACommPort.GetCount() > 0)
    {
        m_comboFPGACommPort.DeleteString(0);
    }

    std::vector<std::string> ports;
    m_instrument.getSerialPorts(&ports);

    for(std::vector<std::string>::iterator i = ports.begin(); i != ports.end(); ++i)
    {
        m_comboFPGACommPort.AddString((*i).c_str());
    }

    if(m_comboFPGACommPort.SelectString(-1, fpgaPort) == CB_ERR)
    {
        m_comboFPGACommPort.SetCurSel(0);
    }
}

void CHerbertControllerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
     DialogEx::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHerbertControllerDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        DialogEx::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHerbertControllerDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CHerbertControllerDlg::OnCancel() 
{
    m_fpgaComm.Close();

    // Window Placement
    WINDOWPLACEMENT dialogPlacement;
    GetWindowPlacement(&dialogPlacement);
    AfxGetApp()->WriteProfileInt(profile_sectionApp, profile_keyDialogXPos, dialogPlacement.rcNormalPosition.left);
    AfxGetApp()->WriteProfileInt(profile_sectionApp, profile_keyDialogYPos, dialogPlacement.rcNormalPosition.top);

    AfxGetApp()->WriteProfileInt(profile_sectionApp, profile_keyTabSelection, m_tabs.GetCurSel());

    CString fpgaPort;
    CString pcbPort;
    m_comboFPGACommPort.GetWindowText(fpgaPort);

    AfxGetApp()->WriteProfileString(profile_sectionComm, profile_keyFPGAPort, fpgaPort);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->OnAbort();
        if(!m_tabDialogs[i]->SaveSettingsAndExit()) return;
    }

    DialogEx::OnCancel();
}



LRESULT CHerbertControllerDlg::OnTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction = m_rspQueue->dequeue();

    if(transaction)
    {
        if(!m_waitingForResponseCount)
        {
            return 0;
        }

        m_waitingForResponseCount--;

        if(transaction->validRsp())
        {
            // Handle successful response
            //switch(transaction->type())
            //{
            //    case Commands::Transaction::TypeGetFpgaFirmwareVersion:
            //    {
            //        Commands::GetFpgaFirmwareVersion* rsp = dynamic_cast<Commands::GetFpgaFirmwareVersion*>(transaction.get());
            //        ITI_ASSERT_NOT_NULL(rsp);

            //        KillTimer(1);

            //        m_staticFPGAFirmwareVersion.SetWindowText(rsp->firmwareVersion());
            //        m_staticFPGAFirmwareBuildDate.SetWindowText(rsp->buildDate());
            //        m_staticFPGAVersion.SetWindowText(rsp->fpgaVersion());
            //        break;
            //    }

            //    default:
            //        // Do nothing
            //        break;
            //}
        }
        else
        {
            // Handle failed transaction
        }
    }

    return 0;
}



LRESULT CHerbertControllerDlg::OnInstrumentEvent(WPARAM wParam, LPARAM lParam)
{
    InstrumentEvent::shared_ptr instrumentEvent = m_instrumentEventQueue->dequeue();
    if(instrumentEvent)
    {
        switch(instrumentEvent->type())
        {
        case InstrumentEvent::Type_ErrorMsg:
            {
                for(int i = 0; i < m_tabDialogs.GetSize(); i++)
                {
                    m_tabDialogs[i]->OnAbort();
                }

                ErrorMessage* errorMsg = dynamic_cast<ErrorMessage*>(instrumentEvent.get());
                ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Error", "%s (%03d)",
                           errorMsg->msg().c_str(), errorMsg->number());
            }
            break;

        case InstrumentEvent::Type_InstrumentException:
            {
                for(int i = 0; i < m_tabDialogs.GetSize(); i++)
                {
                    m_tabDialogs[i]->OnAbort();
                }

                InstrumentException* exception = dynamic_cast<InstrumentException*>(instrumentEvent.get());
                ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Instrument Exception", "%s",
                           exception->msg().c_str());
            }
            break;

        case InstrumentEvent::Type_FPGAInfoUpdate:
            {
                FPGAInfoUpdate* e = dynamic_cast<FPGAInfoUpdate*>(instrumentEvent.get());
                m_staticFPGAFirmwareVersion.SetWindowText(e->firmwareVersion().c_str());
                m_staticFPGAFirmwareBuildDate.SetWindowText(e->firmwareBuildDate().c_str());
                m_staticFPGAVersion.SetWindowText(e->fpgaVersion().c_str());
            }
            break;
        default:
            // Ignore
            break;
        }
    }
    return 0;
}



void CHerbertControllerDlg::OnButtonConnect() 
{
    try
    {
        if(m_instrument.connected())
        {
            for(int i = 0; i < m_tabDialogs.GetSize(); i++)
            {
                m_tabDialogs[i]->OnAbort();
            }
            m_instrument.disconnect();
            UpdateCommPorts();
        }
        else
        {
            CString fpgaPort;
            CString pcbPort;
            m_comboFPGACommPort.GetWindowText(fpgaPort);
            m_instrument.connect(LPCSTR(fpgaPort));
        }
    }
    catch(Instrument::ConnectionException& e)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Connection Error", "%s", e.what());
        UpdateCommPorts();
    }

    DisplayConnected();
}


void CHerbertControllerDlg::DisplayConnected()
{
    bool connected     = m_instrument.connected();
    bool fpgaConnected = m_instrument.fpgaConnected();

    if(connected)
    {
        m_buttonConnect.SetWindowText("Disconnect");
    }
    else
    {
        m_buttonConnect.SetWindowText("Connect");
    }

    m_comboFPGACommPort.EnableWindow(!connected);
    m_buttonStop.EnableWindow(fpgaConnected);

    m_staticFPGAFirmwareVersion.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareBuildDate.EnableWindow(fpgaConnected);
    m_staticFPGAVersion.EnableWindow(fpgaConnected);
    m_staticFPGAVersionLabel.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareVersionLabel.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareBuildDateLabel.EnableWindow(fpgaConnected);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->DisplayFPGAConnected(fpgaConnected);
    }
}

void CHerbertControllerDlg::OnButtonStop() 
{
    Commands::Transaction::shared_ptr cmd(new Commands::Abort());
    m_fpgaComm.sendCommand(m_rspQueue, cmd);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->OnAbort();
    }
}


INT_PTR CHerbertControllerDlg::DoModal()
{
    // TODO: Add your specialized code here and/or call the base class

    ITI::ProcessorException::install();

    std::stringstream errMsg;

    try
    {
        return DialogEx::DoModal();
    }
    catch(ITI::Assert& ex)
    {
        errMsg << "Assert error in Application thread:" << std::endl
               << ex.what() << std::endl << "Application exited";
    }
    catch(ITI::Exception& ex)
    {
        errMsg << "Unhandled exception in Application thread:" << std::endl
               << ex.what() << std::endl << "Stack Trace:" << std::endl
               << ex.stackTrace() << "Application exited";
    }
    catch(std::exception& ex)
    {
        errMsg << "Unhandled exception in Application thread:" << std::endl
               << ex.what() << std::endl << "Application exited";
    }
    catch(...)
    {
        errMsg << "Unhandled unknown exception in Application thread"
               << std::endl << "Application exited";
    }
    m_fpgaComm.Close();
    MessageBox(errMsg.str().c_str(), "Application Error", MB_ICONEXCLAMATION | MB_OK);


    return -1;
}

void CHerbertControllerDlg::OnTimer(UINT_PTR nIDEvent)
{
    DialogEx::OnTimer(nIDEvent);
}