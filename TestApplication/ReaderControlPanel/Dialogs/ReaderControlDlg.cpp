// ReaderControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderControl.h"
#include "ReaderControlDlg.h"
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
const static char profile_keyPCBPort[]          = "PCB Port";
const static char profile_defaultFPGACommPort[] = "COM2";
const static char profile_defaultPCBCommPort[]  = "COM1";



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CStatic m_staticVersionNumber;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Control(pDX, IDC_STATIC_VERSION, m_staticVersionNumber);
    //}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    m_staticVersionNumber.SetWindowText(APP_VERSION_NUMBER_STR);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReaderControlDlg dialog

CReaderControlDlg::CReaderControlDlg(CWnd* pParent /*=NULL*/)
    : DialogEx(CReaderControlDlg::IDD, pParent)
    , m_rspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
    , m_fpgaComm(FpgaComm::getInstance())
    , m_tabDlgManual(this)
    , m_tabDlgPcb(this)
    , m_tabDlgSystem(this)
    , m_waitingForResponseCount(0)
    , m_instrument(Instrument::getInstance())
    , m_instrumentEventQueue(new GuiInstrumentEventQueue(this, WM_INSTRUMENT_EVENT))
{
    //{{AFX_DATA_INIT(CReaderControlDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ITI_LARGE);
}

CReaderControlDlg::~CReaderControlDlg()
{
    m_fpgaComm.Close();
}

void CReaderControlDlg::DoDataExchange(CDataExchange* pDX)
{
    DialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_buttonStop);
    DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
    DDX_Control(pDX, IDC_COMBO_FPGACOMMPORT, m_comboFPGACommPort);
    DDX_Control(pDX, IDC_COMBO_PCBCOMMPORT, m_comboPCBCommPort);
    DDX_Control(pDX, IDC_TABS, m_tabs);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_VER_STR, m_staticFPGAFirmwareVersion);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_BUILD_DATE, m_staticFPGAFirmwareBuildDate);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_VERSION_LABEL, m_staticFPGAFirmwareVersionLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_FW_BUILD_DATE_LABEL, m_staticFPGAFirmwareBuildDateLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_VERSION_LABEL, m_staticFPGAVersionLabel);
    DDX_Control(pDX, IDC_STATIC_FPGA_VER_STR, m_staticFPGAVersion);
    DDX_Control(pDX, IDC_STATIC_PCB_FW_VERSION_LABEL, m_staticPCBFirmwareVersionLabel);
    DDX_Control(pDX, IDC_STATIC_PCB_FW_VER_STR, m_staticPCBFirmwareVersion);
    DDX_Control(pDX, IDC_STATIC_PCB_FW_BUILD_DATE_LABEL, m_staticPCBFirmwareBuildDateLabel);
    DDX_Control(pDX, IDC_STATIC_PCB_FW_BUILD_DATE, m_staticPCBFirmwareBuildDate);
    DDX_Control(pDX, IDC_STATIC_HD_IMAGE_VERSION, m_staticHDImageVersion);
    DDX_Control(pDX, IDC_STATIC_SYSTEM_SERIAL_NUMBER, m_staticSystemSerialNumber);
}

BEGIN_MESSAGE_MAP(CReaderControlDlg, DialogEx)
    //{{AFX_MSG_MAP(CReaderControlDlg)
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
// CReaderControlDlg message handlers

BOOL CReaderControlDlg::OnInitDialog()
{
    DialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
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
//    m_buttonStop.SetFlat(FALSE);
    m_buttonStop.SetWindowText("");


    // Configure Info Display    
    m_staticFPGAFirmwareVersion.SetWindowText("...");
    m_staticFPGAFirmwareBuildDate.SetWindowText("...");
    m_staticFPGAVersion.SetWindowText("...");

    m_staticPCBFirmwareVersion.SetWindowText("...");
    m_staticPCBFirmwareBuildDate.SetWindowText("...");


    // Set Dialog position
    int xPos = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyDialogXPos, -1);
    int yPos = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyDialogYPos, -1);

    SetWindowPos(NULL, xPos, yPos, 0, 0, SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOSIZE);
    

    // Configure Tab Dialogs
    m_tabs.addDialog(m_tabDlgManual, "Manual Control");
    m_tabs.addDialog(m_tabDlgPcb, "PCB Control");
    m_tabs.addDialog(m_tabDlgSystem, "System");

    m_tabDialogs.Add(&m_tabDlgManual);
    m_tabDialogs.Add(&m_tabDlgManual);
    m_tabDialogs.Add(&m_tabDlgPcb);
    m_tabDialogs.Add(&m_tabDlgSystem);
    
    int currentTab = AfxGetApp()->GetProfileInt(profile_sectionApp, profile_keyTabSelection, 0);
    m_tabs.changeTab((currentTab >= 0 && currentTab < m_tabs.GetItemCount()) ? currentTab : 0);


    // Configure Com port
    CString fpgaPort = AfxGetApp()->GetProfileString(profile_sectionComm, profile_keyFPGAPort, profile_defaultFPGACommPort);
    CString pcbPort  = AfxGetApp()->GetProfileString(profile_sectionComm, profile_keyPCBPort, profile_defaultPCBCommPort);

    UpdateCommPorts(fpgaPort, pcbPort);


    m_instrument.watchErrorMessages(m_instrumentEventQueue);
    m_instrument.watchExceptions(m_instrumentEventQueue);
    m_instrument.watchFPGAInfoUpdate(m_instrumentEventQueue);
    m_instrument.watchPCBInfoUpdate(m_instrumentEventQueue);
    
    DisplayConnected();

    m_staticHDImageVersion.SetWindowText(m_instrument.getHDImageVersion().c_str());
    m_staticSystemSerialNumber.SetWindowText(m_instrument.getSystemSerialNumber().c_str());

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReaderControlDlg::UpdateCommPorts()
{
    CString fpgaPort;
    CString pcbPort;
    m_comboFPGACommPort.GetWindowText(fpgaPort);
    m_comboPCBCommPort.GetWindowText(pcbPort);
    UpdateCommPorts(fpgaPort, pcbPort);
}


void CReaderControlDlg::UpdateCommPorts(const CString& fpgaPort, const CString& pcbPort)
{
    while(m_comboFPGACommPort.GetCount() > 0)
    {
        m_comboFPGACommPort.DeleteString(0);
    }
    while(m_comboPCBCommPort.GetCount() > 0)
    {
        m_comboPCBCommPort.DeleteString(0);
    }

    std::vector<std::string> ports;
    m_instrument.getSerialPorts(&ports);

    for(std::vector<std::string>::iterator i = ports.begin(); i != ports.end(); ++i)
    {
        m_comboFPGACommPort.AddString((*i).c_str());
        m_comboPCBCommPort.AddString((*i).c_str());
    }

    if(m_comboFPGACommPort.SelectString(-1, fpgaPort) == CB_ERR)
    {
        m_comboFPGACommPort.SetCurSel(0);
    }

    if(m_comboPCBCommPort.SelectString(-1, pcbPort) == CB_ERR)
    {
        m_comboPCBCommPort.SetCurSel(0);
    }
}

void CReaderControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        DialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CReaderControlDlg::OnPaint() 
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
HCURSOR CReaderControlDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CReaderControlDlg::OnCancel() 
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
    m_comboPCBCommPort.GetWindowText(pcbPort);

    AfxGetApp()->WriteProfileString(profile_sectionComm, profile_keyFPGAPort, fpgaPort);
    AfxGetApp()->WriteProfileString(profile_sectionComm, profile_keyPCBPort, pcbPort);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->OnAbort();
        if(!m_tabDialogs[i]->SaveSettingsAndExit()) return;
    }

    DialogEx::OnCancel();
}



LRESULT CReaderControlDlg::OnTransactionComplete(WPARAM wParam, LPARAM lParam)
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



LRESULT CReaderControlDlg::OnInstrumentEvent(WPARAM wParam, LPARAM lParam)
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

        case InstrumentEvent::Type_PCBInfoUpdate:
            {
                PCBInfoUpdate* e = dynamic_cast<PCBInfoUpdate*>(instrumentEvent.get());
                m_staticPCBFirmwareVersion.SetWindowText(e->firmwareVersion().c_str());
                m_staticPCBFirmwareBuildDate.SetWindowText(e->firmwareBuildDate().c_str());
            }
            break;

        default:
            // Ignore
            break;
        }
    }
    return 0;
}



void CReaderControlDlg::OnButtonConnect() 
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
            m_comboPCBCommPort.GetWindowText(pcbPort);
            m_instrument.connect(LPCSTR(fpgaPort), LPCSTR(pcbPort));
        }
    }
    catch(Instrument::ConnectionException& e)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Connection Error", "%s", e.what());
        UpdateCommPorts();
    }

    DisplayConnected();
}


void CReaderControlDlg::DisplayConnected()
{
    bool connected     = m_instrument.connected();
    bool fpgaConnected = m_instrument.fpgaConnected();
    bool pcbConnected  = m_instrument.pcbConnected();

    if(connected)
    {
        m_buttonConnect.SetWindowText("Disconnect");
    }
    else
    {
        m_buttonConnect.SetWindowText("Connect");
    }

    m_comboFPGACommPort.EnableWindow(!connected);
    m_comboPCBCommPort.EnableWindow(!connected);
    m_buttonStop.EnableWindow(fpgaConnected);

    m_staticFPGAFirmwareVersion.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareBuildDate.EnableWindow(fpgaConnected);
    m_staticFPGAVersion.EnableWindow(fpgaConnected);
    m_staticFPGAVersionLabel.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareVersionLabel.EnableWindow(fpgaConnected);
    m_staticFPGAFirmwareBuildDateLabel.EnableWindow(fpgaConnected);

    m_staticPCBFirmwareVersion.EnableWindow(pcbConnected);
    m_staticPCBFirmwareBuildDate.EnableWindow(pcbConnected);
    m_staticPCBFirmwareVersionLabel.EnableWindow(pcbConnected);
    m_staticPCBFirmwareBuildDateLabel.EnableWindow(pcbConnected);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->DisplayFPGAConnected(fpgaConnected);
        m_tabDialogs[i]->DisplayPCBConnected(pcbConnected);
    }
}

void CReaderControlDlg::OnButtonStop() 
{
    Commands::Transaction::shared_ptr cmd(new Commands::Abort());
    m_fpgaComm.sendCommand(m_rspQueue, cmd);

    for(int i = 0; i < m_tabDialogs.GetSize(); i++)
    {
        m_tabDialogs[i]->OnAbort();
    }
}


INT_PTR CReaderControlDlg::DoModal()
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

void CReaderControlDlg::OnTimer(UINT_PTR nIDEvent)
{
    DialogEx::OnTimer(nIDEvent);
}
