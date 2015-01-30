// DebugCaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include <DebugCaptureDlg.h>
#include <SystemDlg.h>
#include <Messages.h>
#include <StringFormatting.h>
#include <fstream>


// DebugCaptureDlg dialog
const static char profile_sectionDebugCapture[] = "Debug Capture Settings";
const static char profile_keySavePath[]         = "Save Path";

IMPLEMENT_DYNAMIC(DebugCaptureDlg, CDialog)

DebugCaptureDlg::DebugCaptureDlg(CWnd* parent) :
    DialogEx(DebugCaptureDlg::IDD, parent),
    m_parentWindow(parent),
    m_comm(FpgaComm::getInstance()),
    m_rspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE)),
    m_capturing(false)
{
}

DebugCaptureDlg::~DebugCaptureDlg()
{
}

void DebugCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_DEBUG_OUTPUT, m_editDebugOutput);
    DDX_Control(pDX, IDC_BUTTON_CAPTURE_DEBUG_OUTPUT, m_buttonCaptureDebug);
    DDX_Control(pDX, IDC_EDIT_BYTE_COUNT, m_editByteCount);
    DDX_Control(pDX, IDC_PROGRESS_DOWNLOAD, m_progressDownload);
    DDX_Control(pDX, IDC_BUTTON_SAVE, m_buttonSave);
}


BEGIN_MESSAGE_MAP(DebugCaptureDlg, DialogEx)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_CAPTURE_DEBUG_OUTPUT, &DebugCaptureDlg::OnBnClickedButtonCaptureDebugOutput)
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnCommTransactionComplete)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &DebugCaptureDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()

BOOL DebugCaptureDlg::Create()
{
    return DialogEx::Create(IDD, m_parentWindow);
}

// DebugCaptureDlg message handlers

void DebugCaptureDlg::PostNcDestroy()
{
    delete this;
}

void DebugCaptureDlg::OnCancel()
{
    //Ignore
}

void DebugCaptureDlg::OnOK()
{
    //Ignore
}

void DebugCaptureDlg::OnClose()
{
    ((SystemDlg*)m_parentWindow)->CloseDebugCaptureDlg();
    DestroyWindow();
}

INT_PTR DebugCaptureDlg::DoModal()
{
    return DialogEx::DoModal();
}

BOOL DebugCaptureDlg::OnInitDialog()
{
    DialogEx::OnInitDialog();

    CRect parentRect;
    CRect clientRect;
    m_parentWindow->GetClientRect(parentRect);
    ClientToScreen(parentRect);
    GetClientRect(clientRect);

    SetWindowPos(NULL,
                 parentRect.CenterPoint().x - clientRect.CenterPoint().x,
                 parentRect.CenterPoint().y - clientRect.CenterPoint().y,
                 0, 0, SWP_NOSIZE);

    m_editByteCount.SetMaxValue(maxByteCount);
    m_editByteCount.SetValue(defaultByteCount);
    m_buttonSave.EnableWindow(FALSE);

    SetStateIdle();

    return TRUE;
}

void DebugCaptureDlg::OnBnClickedButtonCaptureDebugOutput()
{
    if(!m_capturing)
    {
        SetStateCapturing();
        Commands::Transaction::shared_ptr cmd(new Commands::PauseDebugCapture());
        m_comm.sendCommand(m_rspQueue, cmd);
    }
    else
    {
        SetStateIdle();
        m_editDebugOutput.SetWindowText("");
    }
}

LRESULT DebugCaptureDlg::OnCommTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_rspQueue->dequeue());

    if(!m_capturing)
    {
        return 0;
    }

    if(transaction)
    {
        if(transaction->validRsp() && transaction->getErrorCode() == 0)
        {
            switch(transaction->type())
            {
            case Commands::Transaction::TypePauseDebugCapture:
                {
                    Commands::Transaction::shared_ptr cmd(new Commands::GetDebugCaptureSize());
                    m_comm.sendCommand(m_rspQueue, cmd);
                }
                break;
            case Commands::Transaction::TypeGetDebugCaptureSize:
                {
                    Commands::GetDebugCaptureSize* rsp = dynamic_cast<Commands::GetDebugCaptureSize*>(transaction.get());
                    ITI_ASSERT_NOT_NULL(rsp);

                    unsigned size = rsp->getSize();

                    m_remainingBytesToCapture = __min(m_editByteCount.GetValue(), size);
                    if(m_remainingBytesToCapture < size)
                    {
                        m_captureOffset = size - m_remainingBytesToCapture;
                    }
                    else
                    {
                        m_captureOffset = 0;
                    }

                    m_progressDownload.SetRange32(0, m_remainingBytesToCapture);
                    m_progressDownload.SetPos(0);

                    RequestDebugOutput();
                }
                break;
            case Commands::Transaction::TypeGetDebugCapture:
                {
                    Commands::GetDebugCapture* rsp = dynamic_cast<Commands::GetDebugCapture*>(transaction.get());
                    ITI_ASSERT_NOT_NULL(rsp);

                    unsigned size = rsp->getFormattedDataLength();
                    CString  data = rsp->getFormattedData();
                    m_data += unquoteString(LPCSTR(data)).c_str();

                    m_progressDownload.OffsetPos(size);

                    m_captureOffset += size;
                    if(size < m_remainingBytesToCapture)
                    {
                        m_remainingBytesToCapture -= size;
                        RequestDebugOutput();
                    }
                    else
                    {
                        m_remainingBytesToCapture = 0;
                        Commands::Transaction::shared_ptr cmd(new Commands::ResumeDebugCapture());
                        m_comm.sendCommand(m_rspQueue, cmd);

                        m_progressDownload.SetRange32(0, 100);
                        m_progressDownload.SetPos(100);
                    }
                }
                break;
            case Commands::Transaction::TypeResumeDebugCapture:
                m_editDebugOutput.SetWindowText(m_data);
                m_editDebugOutput.SetSel(0, -1);
                m_editDebugOutput.SetSel(-1, -1);
                SetStateIdle();
                break;
            default:
                break;
            }
        }
        else if(transaction->status().GetLength())
        {
            CString errorMsg = formatString("Status: '%s' (%03d)",
                                            transaction->getErrorString(),
                                            transaction->getErrorCode()).c_str();
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", errorMsg);
            SetStateError("Comm Error: " + errorMsg);
        }
        else
        {
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "'%s'", transaction->getErrorString());
            SetStateError(transaction->getErrorString());
        }
    }

    return 0;
}

void DebugCaptureDlg::SetStateCapturing()
{
    m_capturing = true;
    m_buttonCaptureDebug.SetWindowText("Stop Download");
    m_data.Empty();
    m_editDebugOutput.SetWindowText("Downloading Debug Output...");
    m_buttonSave.EnableWindow(FALSE);
    m_progressDownload.SetPos(0);
}


void DebugCaptureDlg::SetStateIdle()
{
    m_capturing = false;
    m_buttonCaptureDebug.SetWindowText("Download Debug Output");
    m_buttonSave.EnableWindow(!m_data.IsEmpty());
}


void DebugCaptureDlg::SetStateError(const CString& error)
{
    SetStateIdle();

    CString data = m_data;
    if(data.GetLength())
    {
        data += "\r\n";
    }
    
    data += "<Error: " + error + ">";

    m_editDebugOutput.SetWindowText(data);
    m_editDebugOutput.SetSel(0, -1);
    m_editDebugOutput.SetSel(-1, -1);
}


void DebugCaptureDlg::RequestDebugOutput()
{
    Commands::Transaction::shared_ptr cmd(new Commands::GetDebugCapture(m_captureOffset, m_remainingBytesToCapture));
    m_comm.sendCommand(m_rspQueue, cmd);
}

void DebugCaptureDlg::OnBnClickedButtonSave()
{
    CString path = AfxGetApp()->GetProfileString(profile_sectionDebugCapture, profile_keySavePath);
    std::string strFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*|";
    CFileDialog saveFileDialog(false, ".txt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter.c_str());
    saveFileDialog.m_ofn.lpstrTitle = "Save Debug Output To File";
    saveFileDialog.m_ofn.lpstrInitialDir = path.GetBuffer(MAX_PATH);

    if(saveFileDialog.DoModal() == IDOK)
    {
        AfxGetApp()->WriteProfileString(profile_sectionDebugCapture, profile_keySavePath, saveFileDialog.GetPathName());
        SaveOutputToFile(saveFileDialog.GetPathName());
    }
}


void DebugCaptureDlg::SaveOutputToFile(const CString& filename)
{
    std::ofstream outputFile;
    outputFile.exceptions(std::ofstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
    try
    {
        outputFile.open(filename, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
        outputFile << m_data;
    }
    catch (std::ifstream::failure& ex)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "File Write Error", "Error: %s\nFile: %s", ex.what(), filename);
    }
}

