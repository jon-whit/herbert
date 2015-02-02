// FirmwareUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FirmwareUpgradeDlg.h"
#include <Messages.h>
#include <io.h>
#include <Instrument.h>
#include <StringFormatting.h>


const static char profile_sectionFirmwareUpdate[]  = "Firmware Update Settings";
const static char profile_keyFilePath[]            = "File Path";


// FirmwareUpgradeDlg dialog

#define MCS_START_OF_LINE_FLAG       ':'
#define MCS_LINE_TYPE_EXTENDED_ADDR  0x04
#define MCS_LINE_TYPE_END_OF_FILE    0x01
#define MCS_LINE_TYPE_DATA_RECORD    0x00




IMPLEMENT_DYNAMIC(FirmwareUpgradeDlg, CDialog)

FirmwareUpgradeDlg::FirmwareUpgradeDlg(CWnd* pParent /*=NULL*/)
	: DialogEx(FirmwareUpgradeDlg::IDD, pParent)
    , m_running(FALSE)
    , m_closePending(FALSE)
    , m_comm(FpgaComm::getInstance())
    , m_commRspQueue(new TransactionGuiMessagingThreadQueue(this, WM_COMM_TRANSACTION_COMPLETE))
    , m_upgradeFile(NULL)
    , m_upgradeType(UpgradeFirmware)
{

}

FirmwareUpgradeDlg::~FirmwareUpgradeDlg()
{
}

void FirmwareUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
    DialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS, m_progressBar);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
    DDX_Control(pDX, IDC_BUTTON_START, m_buttonStart);
}


BEGIN_MESSAGE_MAP(FirmwareUpgradeDlg, DialogEx)
    ON_BN_CLICKED(IDC_BUTTON_START, &FirmwareUpgradeDlg::OnBnClickedButtonStart)
    ON_BN_CLICKED(IDCANCEL, &FirmwareUpgradeDlg::OnBnClickedCancel)
    ON_WM_TIMER()
    ON_MESSAGE(WM_COMM_TRANSACTION_COMPLETE, OnTransactionComplete)
END_MESSAGE_MAP()


// FirmwareUpgradeDlg message handlers


INT_PTR FirmwareUpgradeDlg::DoModal(UpgradeType upgradeType)
{
    m_upgradeType = upgradeType;

    return DialogEx::DoModal();
}




BOOL FirmwareUpgradeDlg::OnInitDialog()
{
    SetWindowText((m_upgradeType == UpgradeFirmware) ? 
        "Herbert FPGA Firmware Upgrade" : "Herbert FPGA Image Upgrade");

    DialogEx::OnInitDialog();

    ResetState();

    return TRUE;
}



LRESULT FirmwareUpgradeDlg::OnTransactionComplete(WPARAM wParam, LPARAM lParam)
{
    Commands::Transaction::shared_ptr transaction(m_commRspQueue->dequeue());

    if(transaction)
    {
        if(m_closePending)
        {
            OnCancel();
        }

        if(transaction->validRsp())
        {
            if(transaction->statusOk())
            {
                if(transaction->type() == Commands::Transaction::TypeSendFileData)
                {
                    SendFileNext();
                }
                else if (transaction->type() == Commands::Transaction::TypeUpgrade)
                {
                    ResetState();
                    Instrument::updateFPGAInfo();

                    m_staticStatus.SetWindowText("Upgrade complete");
                    ShowMsgBox(MB_ICONINFORMATION | MB_OK, "Upgrade Complete",
                               "Firmware image has been uploaded and verified\n"
                               "Transferred in %d seconds\n"
                               "Programming image to flash...",
                               time(NULL) - m_startTime);
                    OnCancel();
                }
                else
                {
                    KillTimer(1);
                    ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Upgrade Error", "Upgrade error");
                    ResetState();
                    m_staticStatus.SetWindowText("Upgrade Error");
                }
            }
            else
            {
                CString msg;
                KillTimer(1);

                msg.Format("%s (Code %d)", transaction->getErrorString(), transaction->getErrorCode());
                ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Upgrade Error", "An error has occured:\n" + msg);
                ResetState();
                m_staticStatus.SetWindowText("Upgrade Error: " + msg);
            }
        }
        else if(transaction->status().GetLength())
        {
            KillTimer(1);
            m_running = FALSE;
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "Status: '%s'", transaction->status());
            ResetState();
            m_staticStatus.SetWindowText("Comm Error");
        }
        else
        {
            KillTimer(1);
            m_running = FALSE;
            ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Comm Error", "'%s'", transaction->getErrorString());
            ResetState();
            m_staticStatus.SetWindowText("Comm Error");
        }
    }

    return 0;
}

void FirmwareUpgradeDlg::ResetState()
{
    KillTimer(1);
    m_running = FALSE;
    m_buttonStart.SetWindowText("Open File");
    m_staticStatus.SetWindowText("");
}



void FirmwareUpgradeDlg::OnBnClickedButtonStart()
{
    if(!m_running)
    {
        CString initialPath = AfxGetApp()->GetProfileString(profile_sectionFirmwareUpdate, profile_keyFilePath);
        CFileDialog upgradeFileDialog(true, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST,
            (m_upgradeType == UpgradeFirmware) ?
            "Binary Firmware Image (*.bin)|*.bin|All Files (*.*)|*.*||" :
            "Xilinx FPGA MCS Image (*.mcs)|*.mcs|All Files (*.*)|*.*||");
        upgradeFileDialog.m_ofn.lpstrTitle = "Open Upgrade File";
        upgradeFileDialog.m_ofn.lpstrInitialDir = initialPath.GetBuffer(MAX_PATH);

        if(upgradeFileDialog.DoModal() != IDOK)
        {
            ResetState();
            return;
        }

        AfxGetApp()->WriteProfileString(profile_sectionFirmwareUpdate, profile_keyFilePath, upgradeFileDialog.GetPathName());

        m_fileBuffer.clear();
        m_fileLineCount = 0;
        m_running = TRUE;
        m_buttonStart.SetWindowText("Cancel Upgrade");

        m_upgradeFileName = upgradeFileDialog.GetPathName();
        SendFileBegin(m_upgradeFileName);
    }
    else
    {
        ResetState();
        m_staticStatus.SetWindowText("Upgrade cancelled");
    }
}



void FirmwareUpgradeDlg::OnBnClickedCancel()
{
    if(m_running)
    {
        ResetState();
        m_closePending = TRUE;
        m_staticStatus.SetWindowText("Closing...");

        Commands::Transaction::shared_ptr transaction(m_commRspQueue->dequeue());

        if(transaction)
        {
            OnCancel();
        }
    }
    else
    {
        OnCancel();
    }
}


void FirmwareUpgradeDlg::OnTimer(UINT_PTR nIDEvent)
{
    KillTimer(1);
    if(m_running)
    {
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "Upgrade Error", "Upgrade timed out");
    }

    ResetState();
    m_staticStatus.SetWindowText("Upgrade Timed Out");

    DialogEx::OnTimer(nIDEvent);
}


void FirmwareUpgradeDlg::SendFileBegin(CString& filename)
{
    if(m_upgradeFile)
    {
        fclose(m_upgradeFile);
    }

    m_progressBar.SetPos(0);

    if(!(m_upgradeFile = fopen(filename, "rb")))
    {
        ShowMsgBox(MB_OK | MB_ICONEXCLAMATION, "Upgrade File Open Error", "Unable to open upgrade\nfile %s", filename);
        throw Exception("File read error");
    }
    else
    {
        m_staticStatus.SetWindowText("Uploading file: " + filename);

        m_upgradeFileIndex  = 0;
        m_upgradeFileLength = _filelength(_fileno(m_upgradeFile));
        m_upgradeFileCRC    = INITIAL_CRC32_VALUE;
        m_progressBar.SetRange32(0, m_upgradeFileLength);

        m_startTime = time(NULL);
        SendFileNext();
    }
}



void FirmwareUpgradeDlg::SendFileNext()
{
    try
    {
        if(!m_upgradeFile)
        {
            throw Exception("File Not Open");
        }

        if(!m_running)
            return;

        if(feof(m_upgradeFile))
        {
            //File complete
            HandleEndOfFile();
        }
        else
        {
            unsigned count;
            
            if(m_upgradeType == UpgradeFirmware)
            {
                count = SendNextFirmwareFileData();
            }
            else
            {
                count = SendNextFPGAFileData();
            }
            m_upgradeFileIndex += count;
            m_progressBar.SetPos(ftell(m_upgradeFile));

            if(count == 0)
            {
                HandleEndOfFile();
            }
        }
    }
    catch(Exception& ex)
    {
        ResetState();
        fclose(m_upgradeFile);
        m_upgradeFile = NULL;
        ShowMsgBox(MB_ICONEXCLAMATION | MB_OK, "File Send Error", "Error sending file:\n%s", ex.what());
    }
}


unsigned FirmwareUpgradeDlg::SendNextFirmwareFileData()
{
    const unsigned MAX_BIN_DATA_SIZE = MAX_FILE_DATA_SIZE / 2;
    char buf[MAX_BIN_DATA_SIZE];

    unsigned count = fread(buf, 1, MAX_BIN_DATA_SIZE, m_upgradeFile);

    if(count > 0)
    {
        m_upgradeFileCRC = calcCRC32(m_upgradeFileCRC, buf, count);

        SetTimer(1, UPGRADE_TIMEOUT_ms, NULL);
        Commands::Transaction::shared_ptr cmd(new
            Commands::SendFileData(m_upgradeFileIndex, buf, count));
        m_comm.sendCommand(m_commRspQueue, cmd);
    }

    return count;
}


unsigned FirmwareUpgradeDlg::HexCharToValue(char c)
{
    unsigned value;
    if(c >= '0' && c <= '9')
    {
        value = c - '0';
    }
    else if(c >= 'A' && c <= 'F')
    {
        value = c - 'A' + 0xA;
    }
    else
    {
        std::stringstream s;
        s << "Invalid file data on line " << m_fileLineCount;
        throw Exception(s.str());
    }

    return value;
}


unsigned FirmwareUpgradeDlg::GetNextHexValue(std::string& string, unsigned bytes, char* chksum)
{
    if(string.size() < bytes * 2)
    {
        std::stringstream s;
        s << "Missing file data on line " << m_fileLineCount;
        throw Exception(s.str());
    }

    unsigned value = 0;

    while(bytes > 0)
    {
        value <<= 8;
        --bytes;

        unsigned byte = (HexCharToValue(string[0]) << 4) + HexCharToValue(string[1]);
        
        if(chksum)
        {
            *chksum += byte;
        }

        value += byte;
        string.erase(0, 2);
    }

    return value;
}


unsigned FirmwareUpgradeDlg::SendNextFPGAFileData()
{
    const unsigned MAX_BIN_DATA_SIZE = MAX_FILE_DATA_SIZE / 2;

    // Load file buffer
    while(m_fileBuffer.size() < MAX_BIN_DATA_SIZE)
    {
        const int MAX_LINE_LENGTH = 1000;
        std::string line;

        line.resize(MAX_LINE_LENGTH);

        if(!fgets(&line[0], MAX_LINE_LENGTH, m_upgradeFile))
        {
            if(ferror(m_upgradeFile))
            {
                std::stringstream s;
                s << "File error on line " << m_fileLineCount << "(" << errno << ") " << _strerror(NULL);
                throw Exception(s.str());
            }

            break;
        }

        
        unsigned count = line.find('\0');
            
        line.resize(count);
        line = trimString(line);

        if(line.size() == 0)
        {
            break;
        }
        ++m_fileLineCount;

        if(line[0] != MCS_START_OF_LINE_FLAG)
        {
            std::stringstream s;
            s << "Invalid file data on line " << m_fileLineCount;
            throw Exception(s.str());
        }

        line.erase(0, 1);

        char     calcChkSum = 0;

        unsigned dataCount = GetNextHexValue(line, 1, &calcChkSum);
        unsigned addr      = GetNextHexValue(line, 2, &calcChkSum);
        unsigned type      = GetNextHexValue(line, 1, &calcChkSum);

        switch(type)
        {
        case MCS_LINE_TYPE_EXTENDED_ADDR:
        {
            unsigned extendedAddr = GetNextHexValue(line, 2, &calcChkSum);

            if((m_upgradeFileIndex + m_fileBuffer.size()) != (extendedAddr << 16))
            {
                std::stringstream s;
                s << "Invalid extended address on line " << m_fileLineCount;
                throw Exception(s.str());
            }

            break;
        }

        case MCS_LINE_TYPE_END_OF_FILE:
            // End of image - continue to do one more read and get the EOF
            continue;

        case MCS_LINE_TYPE_DATA_RECORD:
            if(((m_upgradeFileIndex + m_fileBuffer.size()) & 0x0000ffff) != addr)
            {
                std::stringstream s;
                s << "Invalid address on line " << m_fileLineCount;
                throw Exception(s.str());
            }

            for(unsigned i = 0; i < dataCount; ++i)
            {
                m_fileBuffer.push_back(GetNextHexValue(line, 1, &calcChkSum));
            }
            break;

        default:
            std::stringstream s;
            s << "Invalid record type on line " << m_fileLineCount;
            throw Exception(s.str());
        }

        unsigned lineChkSum = GetNextHexValue(line, 1);

        if(((lineChkSum + calcChkSum) & 0x000000ff) != 0)
        {
            throw Exception(formatString("Invalid line chksum (calc 0x%02X, line 0x%02X) on line %u",
                                         static_cast<unsigned>(-calcChkSum) & 0x000000ff,
                                         lineChkSum, m_fileLineCount));
        }
    }


    //Send file buffer
    unsigned sendCount = __min(MAX_BIN_DATA_SIZE, m_fileBuffer.size());

    if(sendCount > 0)
    {
        m_upgradeFileCRC = calcCRC32(m_upgradeFileCRC, &m_fileBuffer.at(0), sendCount);

        SetTimer(1, UPGRADE_TIMEOUT_ms, NULL);
        Commands::Transaction::shared_ptr cmd(new
            Commands::SendFileData(m_upgradeFileIndex, &m_fileBuffer.at(0), sendCount));
        m_comm.sendCommand(m_commRspQueue, cmd);

        m_fileBuffer.erase(m_fileBuffer.begin(), m_fileBuffer.begin() + sendCount);
    }

    return sendCount;
}


 



void FirmwareUpgradeDlg::HandleEndOfFile()
{
    if(ferror(m_upgradeFile))
    {
        ResetState();
        m_staticStatus.SetWindowText("Error reading upgrade file");
        ShowMsgBox(MB_OK | MB_ICONEXCLAMATION, "Error", "Error reading upgrade file");
    }
    else if(feof(m_upgradeFile))
    {
        //File complete
        m_staticStatus.SetWindowText("Verifying upgrade image...");
        SetTimer(1, UPGRADE_TIMEOUT_ms, NULL);

        if(m_upgradeType == UpgradeFirmware)
        {
            if(m_upgradeFileLength != m_upgradeFileIndex)
            {
                throw Exception("File length incorrect");
            }

            Commands::Transaction::shared_ptr cmd(new
                Commands::UpgradeFirmware(m_upgradeFileIndex, m_upgradeFileCRC));
            m_comm.sendCommand(m_commRspQueue, cmd);
        }
        else
        {
            Commands::Transaction::shared_ptr cmd(new
                Commands::UpgradeFPGA(m_upgradeFileIndex, m_upgradeFileCRC));
            m_comm.sendCommand(m_commRspQueue, cmd);
        }
        m_progressBar.SetPos(m_upgradeFileLength);
    }
    else
    {
        throw Exception("Unknown error");
    }

    fclose(m_upgradeFile);
    m_upgradeFile = NULL;
}



//EOF

