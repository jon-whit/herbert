// SystemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SystemDlg.h"
#include <FirmwareUpgradeDlg.h>
#include <Messages.h>
#include <StringFormatting.h>
#include <boost/bind.hpp>


const std::string rachisConfigFrameName = "Olympia Server Configuration";



// SystemDlg dialog

IMPLEMENT_DYNAMIC(SystemDlg, CDialog)

SystemDlg::SystemDlg(CWnd* pParent /*=NULL*/)
	: ReaderControlTabDlg(SystemDlg::IDD, pParent)
    , m_debugCaptureDlg(NULL)
    , m_fpgaComm(FpgaComm::getInstance())
    , m_pcbComm(PcbComm::getInstance())
    , m_commRspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
    , m_monitorConnection(false)
{
}


SystemDlg::~SystemDlg()
{
}


void SystemDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_UPGRADE_FIRMWARE, m_buttonUpgradeFirmware);
    DDX_Control(pDX, IDC_BUTTON_CALIBRATION, m_buttonCalibration);
    DDX_Control(pDX, IDC_BUTTON_INIT_SYSTEM, m_buttonInitSystem);
    DDX_Control(pDX, IDC_BUTTON_OPEN_DOOR, m_buttonOpenDoor);
    DDX_Control(pDX, IDC_BUTTON_CLOSE_DOOR, m_buttonCloseDoor);
    DDX_Control(pDX, IDC_BUTTON_SELF_TEST, m_buttonRunSelfTest);
    DDX_Control(pDX, IDC_BUTTON_UPGRADE_FPGA, m_buttonUpgradeFPGA);
    DDX_Control(pDX, IDC_BUTTON_DEVICE_TEST, m_buttonDeviceTest);
    DDX_Control(pDX, IDC_STATIC_RACHIS_CONFIG_FRAME, m_staticRachisConfigFrame);
    DDX_Control(pDX, IDC_CHECK_MONITOR_CONNECTION, m_checkMonitorConnection);
    DDX_Control(pDX, IDC_STATIC_PCR_MASK_CONFIGURATION, m_staticPcrMaskConfiguration);
    DDX_Control(pDX, IDC_STATIC_LOG_LEVEL, m_staticLogLevel);
    DDX_Control(pDX, IDC_STATIC_RUN_IMAGE_CAPTURE_TYPE, m_staticRunImageCaptureType);
    DDX_Control(pDX, IDC_STATIC_DATA_RETENTION_TIME, m_staticDataRetentionTime);
    DDX_Control(pDX, IDC_STATIC_GEO_CORRECTION, m_staticGeoCorrection);
    DDX_Control(pDX, IDC_STATIC_SHUTDOWN_TIME, m_staticShutdownTime);
    DDX_Control(pDX, IDC_BUTTON_SAVE_CONFIGURATION, m_buttonSaveConfiguration);
    DDX_Control(pDX, IDC_BUTTON_FILE_NAMING_TOOL, m_buttonFileNamingTool);
    DDX_Control(pDX, IDC_BUTTON_DEBUG_CAPTURE, m_buttonDebugCapture);
}


BEGIN_MESSAGE_MAP(SystemDlg, ReaderControlTabDlg)
    ON_BN_CLICKED(IDC_BUTTON_UPGRADE_FIRMWARE, &SystemDlg::OnBnClickedButtonUpgradeFirmware)
    ON_BN_CLICKED(IDC_BUTTON_CALIBRATION, &SystemDlg::OnBnClickedButtonCalibration)
    ON_BN_CLICKED(IDC_BUTTON_INIT_SYSTEM, &SystemDlg::OnBnClickedButtonInitSystem)
    ON_BN_CLICKED(IDC_BUTTON_OPEN_DOOR, &SystemDlg::OnBnClickedButtonOpenDoor)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE_DOOR, &SystemDlg::OnBnClickedButtonCloseDoor)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnTransactionComplete)
    ON_MESSAGE(WM_RACHIS_CONFIG_UPDATE, OnRachisConfigUpdate)
    ON_BN_CLICKED(IDC_BUTTON_SELF_TEST, &SystemDlg::OnBnClickedButtonSelfTest)
    ON_BN_CLICKED(IDC_BUTTON_UPGRADE_FPGA, &SystemDlg::OnBnClickedButtonUpgradeFpga)
    ON_BN_CLICKED(IDC_BUTTON_DEVICE_TEST, &SystemDlg::OnBnClickedButtonDeviceTest)
    ON_BN_CLICKED(IDC_BUTTON_DEBUG_CAPTURE, &SystemDlg::OnBnClickedButtonDebugCapture)
    ON_BN_CLICKED(IDC_CHECK_MONITOR_CONNECTION, &SystemDlg::OnBnClickedCheckMonitorConnection)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIGURATION, &SystemDlg::OnBnClickedButtonSaveConfiguration)
    ON_BN_CLICKED(IDC_BUTTON_FILE_NAMING_TOOL, &SystemDlg::OnBnClickedButtonFileNamingTool)
END_MESSAGE_MAP()


// SystemDlg message handlers
BOOL SystemDlg::OnInitDialog()
{
    ReaderControlTabDlg::OnInitDialog();

    m_dlgControls.Add(&m_buttonUpgradeFirmware);
    m_dlgControls.Add(&m_buttonUpgradeFPGA);
    m_dlgControls.Add(&m_buttonCalibration);
    m_dlgControls.Add(&m_buttonInitSystem);
    m_dlgControls.Add(&m_buttonOpenDoor);
    m_dlgControls.Add(&m_buttonCloseDoor);
    m_dlgControls.Add(&m_buttonRunSelfTest);
    m_dlgControls.Add(&m_buttonFileNamingTool);
    m_dlgControls.Add(&m_buttonDebugCapture);

    SetIdle();

    ShowUnconfigured("No config file");

    return TRUE;
}


LRESULT SystemDlg::OnTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_commRspQueue->dequeue());

    if(transaction)
    {
        if(transaction->validRsp())
        {
            if(!transaction->statusOk())
            {
                ShowMsgBox(MB_ICONINFORMATION | MB_OK, "Error",
                    "%s (Code %03d)", transaction->getErrorString(), transaction->getErrorCode());
            }
            else
            {
                switch(transaction->type())
                {
                case Commands::Transaction::TypeSelfTest:
                    ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Self Test Compete", "Self Test completed successfully");
                    break;

                default:
                    //Do nothing
                    break;
                }
            }
        }
        else if(transaction->status().GetLength())
        {
            OnAbort();
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "Status: '%s'", transaction->status());
        }
        else
        {
            OnAbort();
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "'%s'", transaction->getErrorString());
        }

        SetIdle();
    }

    return 0;
}
LRESULT SystemDlg::OnRachisConfigUpdate(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void SystemDlg::ShowUnconfigured(const std::string& errorMsg)
{
    m_staticRachisConfigFrame.SetWindowText((rachisConfigFrameName + " (" + errorMsg + ")").c_str());

    m_checkMonitorConnection.SetCheck(0);
    m_staticPcrMaskConfiguration.SetWindowText("Unknown");
    m_staticLogLevel.SetWindowText("Unknown");
    m_staticRunImageCaptureType.SetWindowText("Unknown");
    m_staticDataRetentionTime.SetWindowText("Unknown");
    m_staticGeoCorrection.SetWindowText("Unknown");
    m_staticShutdownTime.SetWindowText("Unknown");

    m_checkMonitorConnection.EnableWindow(false);
    m_staticPcrMaskConfiguration.ChangeFontStyle(FC_FONT_ITALIC, FC_FONT_BOLD, true);
    m_staticLogLevel.ChangeFontStyle(FC_FONT_ITALIC, 0, true);
    m_staticRunImageCaptureType.ChangeFontStyle(FC_FONT_ITALIC, 0, true);
    m_staticDataRetentionTime.ChangeFontStyle(FC_FONT_ITALIC, 0, true);
    m_staticGeoCorrection.ChangeFontStyle(FC_FONT_ITALIC, 0, true);
    m_staticShutdownTime.ChangeFontStyle(FC_FONT_ITALIC, 0, true);
    m_buttonSaveConfiguration.EnableWindow(false);
}

void SystemDlg::OnAbort()
{
    SetIdle();
}


BOOL SystemDlg::SaveSettingsAndExit()
{
    return TRUE;
}


void SystemDlg::SetIdle()
{
    m_buttonInitSystem.SetWindowText("Initialize System");
    m_buttonOpenDoor.SetWindowText("Open Door");
    m_buttonCloseDoor.SetWindowText("Close Door");
}



void SystemDlg::DisplayFPGAConnected(BOOL connected)
{
    for(int i = 0; i < m_dlgControls.GetSize(); i++)
    {
        m_dlgControls[i]->EnableWindow(connected);
    }

    m_buttonDeviceTest.EnableWindow(m_fpgaComm.IsOpen() || m_pcbComm.IsOpen());

    SetIdle();
}

void SystemDlg::DisplayPCBConnected(BOOL connected)
{
    m_buttonDeviceTest.EnableWindow(m_fpgaComm.IsOpen() || m_pcbComm.IsOpen());
}

void SystemDlg::OnBnClickedButtonUpgradeFirmware()
{
    FirmwareUpgradeDlg dlg(m_pParentWnd);
    dlg.DoModal(FirmwareUpgradeDlg::UpgradeFirmware);
}

void SystemDlg::OnBnClickedButtonUpgradeFpga()
{
    FirmwareUpgradeDlg dlg(m_pParentWnd);
    dlg.DoModal(FirmwareUpgradeDlg::UpgradeFPGA);
}


void SystemDlg::OnBnClickedButtonCalibration()
{
}


void SystemDlg::OnBnClickedButtonInitSystem()
{
    m_buttonInitSystem.SetWindowText("Initializing System...");
    Commands::Transaction::shared_ptr cmd(new Commands::InitSystem());
    m_fpgaComm.sendCommand(m_commRspQueue, cmd);
}

void SystemDlg::OnBnClickedButtonOpenDoor()
{
    m_buttonOpenDoor.SetWindowText("Opening Door...");
    Commands::Transaction::shared_ptr cmd(new Commands::OpenDoor());
    m_fpgaComm.sendCommand(m_commRspQueue, cmd);
}

void SystemDlg::OnBnClickedButtonCloseDoor()
{
    m_buttonCloseDoor.SetWindowText("Closing Door...");
    Commands::Transaction::shared_ptr cmd(new Commands::CloseDoor());
    m_fpgaComm.sendCommand(m_commRspQueue, cmd);
}

void SystemDlg::OnBnClickedButtonSelfTest()
{
}

void SystemDlg::CloseSelfTestDlg()
{

}

void SystemDlg::OnBnClickedButtonFileNamingTool()
{

}

void SystemDlg::CloseFileNamingToolDlg()
{

}

void SystemDlg::OnBnClickedButtonDeviceTest()
{

}

void SystemDlg::CloseDeviceTestDlg()
{

}


void SystemDlg::OnBnClickedButtonDebugCapture()
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


void SystemDlg::CloseDebugCaptureDlg()
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


void SystemDlg::UpdateSaveConfigurationButton()
{
    if((m_checkMonitorConnection.GetCheck()  && !m_monitorConnection) ||
       (!m_checkMonitorConnection.GetCheck() && m_monitorConnection))
    {
        m_buttonSaveConfiguration.EnableWindow();
    }
    else
    {
        m_buttonSaveConfiguration.EnableWindow(false);
    }
}


void SystemDlg::OnBnClickedCheckMonitorConnection()
{
    UpdateSaveConfigurationButton();
}

void SystemDlg::OnBnClickedButtonSaveConfiguration()
{
    if((m_checkMonitorConnection.GetCheck()  && !m_monitorConnection) ||
       (!m_checkMonitorConnection.GetCheck() && m_monitorConnection))
    {
        m_monitorConnection = !!m_checkMonitorConnection.GetCheck();
    }

    UpdateSaveConfigurationButton();
}
