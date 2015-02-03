/////////////////////////////////////////////////////////////
//
//  Commands.cpp
//
//  Plate Cycler Comm Protocol Commands
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <Commands.h>
#include <StringFormatting.h>
#include <boost/lexical_cast.hpp>


//----------------------------------------------------------------

const char* Commands::ConvertModelParam(Commands::ModelParam modelParam)
{
    ITI_ASSERT_LESS(modelParam, ModelParam_Count);
    return MODEL_PARAMS[modelParam];
}

Commands::ModelParam Commands::ConvertModelParam(const std::string& modelParam)
{
    for(int i = 0; i < ModelParam_Count; ++i)
    {
        if(modelParam == MODEL_PARAMS[i])
        {
            return static_cast<ModelParam>(i);
        }
    }

    std::stringstream s;
    s << "Unknown ModelParam: '" << modelParam << "'";
    ITI_ERROR(s.str().c_str());

    return ModelParam_Count;
}

//----------------------------------------------------------------


const char* Commands::ConvertWParam(Commands::WParam wParam)
{
    ITI_ASSERT_LESS(wParam, WParam_Count);
    return W_PARAMS[wParam];
}

Commands::WParam Commands::ConvertWParam(const std::string& wParam)
{
    for(int i = 0; i < WParam_Count; ++i)
    {
        if(wParam == W_PARAMS[i])
        {
            return static_cast<WParam>(i);
        }
    }

    std::stringstream s;
    s << "Unknown wParam: '" << wParam << "'";
    ITI_ERROR(s.str().c_str());

    return WParam_Count;
}



//----------------------------------------------------------------



Commands::Transaction::Transaction(const CString& Header, BOOL setHeader /*= TRUE */)
    : m_validRsp(FALSE)
    , m_isOfflineRsp(FALSE)
    , m_errorCode(0)
{
    if(setHeader)
    {
        m_cmdPkt.SetHeader(Header);
        m_cmdPkt.InitSeqNum();
    }
}



BOOL Commands::Transaction::validateRsp(CommPkt* rspPkt)
{
    if(m_cmdPkt.GetCmd() != rspPkt->GetCmd()) return FALSE;

    #if USE_SEQ_NUMBER
        if(m_cmdPkt.GetSeqNum() != rspPkt->GetSeqNum() ) return FALSE;
    #endif

    if(rspPkt->GetStatus() == COMM_STATUS_PENDING)
    {
        return TRUE;
    }
    
    if(rspPkt->GetStatus() == COMM_STATUS_INFO)
    {
        return TRUE;
    }
    
    if(rspPkt->GetStatus() == COMM_STATUS_OK)
    {
        m_validRsp = processRsp(rspPkt);

        if(m_validRsp)
        {
            m_rspString = rspPkt->GetOriginalPktStr(); 
            m_statusStr = rspPkt->GetStatus();
        }
        else
        {
            CString error;
            error.Format("Invalid response pkt: %s", rspPkt->GetOriginalPktStr());
            setErrorString(error);
        }

        return TRUE;
    }


    if(rspPkt->GetStatus() == COMM_STATUS_ERROR && rspPkt->GetDataLen() > 1)
    {
        m_validRsp = TRUE;
        m_rspString = rspPkt->GetOriginalPktStr(); 
        m_statusStr = rspPkt->GetStatus();
        m_errorCode = rspPkt->GetErrorCode();
        m_errorString = rspPkt->GetErrorString();
        return m_validRsp;
    }

    return FALSE;
}


void Commands::Transaction::setResonse(const CString& statusStr, int errorCode, const CString& errorString)
{
    m_validRsp    = true;
    m_statusStr   = statusStr;
    m_errorCode   = errorCode;
    m_errorString = errorString;
    m_rspString.Empty();
}


BOOL Commands::Transaction::isErrorMessage(CommPkt* rspPkt)
{
    return rspPkt->GetSeqNum() == "00" &&
           rspPkt->GetCmd()    == ERROR_MSG_KEY_STR &&
           rspPkt->GetStatus() == COMM_STATUS_INFO;
}

BOOL Commands::Transaction::isLogMessage(CommPkt* rspPkt)
{
    return rspPkt->GetSeqNum() == "00" &&
           rspPkt->GetCmd()    == LOG_MSG_KEY_STR &&
           rspPkt->GetStatus() == COMM_STATUS_INFO;
}






Commands::Manual::Manual(const CString& header, const CString& cmd, BOOL addHeader)
    : Transaction(header, addHeader)
{
    m_cmdPkt.AddData(cmd);
}

BOOL Commands::Manual::processRsp(CommPkt* rspPkt)
{
    for(int i = 1; i < rspPkt->GetDataLen(); i++)
    {
        m_rspParams.push_back(rspPkt->GetData(i));
    }
    return true;
}




Commands::FpgaManual::FpgaManual(const CString& cmd, BOOL addHeader)
    : Manual(HEADER_TO_MASTER_FROM_HOST, cmd, addHeader)
{
}

Commands::Abort::Abort()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(ABORT_STR);
}

BOOL Commands::Abort::processRsp(CommPkt* rspPkt)
{
    return true;
}

//----------------------------------------------------------------------------

Commands::GetFirmwareVersion::GetFirmwareVersion(const CString& header)
    : Transaction(header)
{
    m_cmdPkt.SetCmd(GET_FW_VERSION_STR);
}

BOOL Commands::GetFirmwareVersion::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() < 2)
    {
        return false;
    }

    m_firmwareVersion = rspPkt->GetData(1);

    for(int i = 2; i < rspPkt->GetDataLen(); i++)
    {
        if(m_buildDate.GetLength())
        {
            m_buildDate += " ";
        }
        m_buildDate += rspPkt->GetData(i);
    }

    return true;
}



Commands::GetFpgaFirmwareVersion::GetFpgaFirmwareVersion()
    : GetFirmwareVersion(HEADER_TO_MASTER_FROM_HOST)
{
}



Commands::GetPcbFirmwareVersion::GetPcbFirmwareVersion()
    : GetFirmwareVersion(HEADER_TO_PRB_FROM_SBC)
{
}

//----------------------------------------------------------------------------

Commands::GetFpgaVersion::GetFpgaVersion()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_FPGA_VERSION_STR);
}

BOOL Commands::GetFpgaVersion::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() < 2)
    {
        return false;
    }

    m_fpgaVersion = rspPkt->GetData(1);
    return true;
}

//----------------------------------------------------------------------------

Commands::GetSerialNumber::GetSerialNumber(const CString& header)
    : Transaction(header)
{
    m_cmdPkt.SetCmd(GET_SERIAL_NUMBER_STR);
}

BOOL Commands::GetSerialNumber::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() < 2)
    {
        return false;
    }

    m_serialNumber = rspPkt->GetData(1);
    return true;
}



Commands::GetFpgaSerialNumber::GetFpgaSerialNumber()
    : GetSerialNumber(HEADER_TO_MASTER_FROM_HOST)
{
}



Commands::GetPcbSerialNumber::GetPcbSerialNumber()
    : GetSerialNumber(HEADER_TO_PRB_FROM_SBC)
{
}

//----------------------------------------------------------------------------

Commands::GetTemperatureBlock::GetTemperatureBlock()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_CURRENT_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_BLOCK);
}

BOOL Commands::GetTemperatureBlock::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 2 + CHANNEL_COUNT)
    {
        for(int i = 0; i < CHANNEL_COUNT; i++)
        {
            m_temp[i] = rspPkt->GetData(2 + i);
        }
        return true;
    }
    else
    {
        return false;
    }
}





Commands::GetTemperatureLid::GetTemperatureLid()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_CURRENT_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_LID);
}

BOOL Commands::GetTemperatureLid::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 4)
    {
        m_temp = rspPkt->GetData(2);
        return true;
    }
    else
    {
        return false;
    }
}





Commands::GetTemperatureHeatsink::GetTemperatureHeatsink()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_CURRENT_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_HEATSINK);
}

BOOL Commands::GetTemperatureHeatsink::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 3)
    {
        m_temp = rspPkt->GetData(2);
        return true;
    }
    else
    {
        return false;
    }
}





Commands::GetTemperatureAmbient::GetTemperatureAmbient()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_CURRENT_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_AMBIENT);
}

BOOL Commands::GetTemperatureAmbient::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 3)
    {
        m_temp = rspPkt->GetData(2);
        return true;
    }
    else
    {
        return false;
    }
}



Commands::GetTemperatureAll::GetTemperatureAll()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_CURRENT_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_ALL);
}

BOOL Commands::GetTemperatureAll::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 2 + CHANNEL_COUNT + 4)
    {
        for(int i = 0; i < CHANNEL_COUNT; i++)
        {
            m_blockTemps[i] = rspPkt->GetData(2 + i);
        }

        m_lidCenterTemp = rspPkt->GetData(2 + CHANNEL_COUNT + 0);
        m_lidRingTemp   = rspPkt->GetData(2 + CHANNEL_COUNT + 1);
        m_ambientTemp   = rspPkt->GetData(2 + CHANNEL_COUNT + 2);
        m_heatsinkTemp  = rspPkt->GetData(2 + CHANNEL_COUNT + 3);

        return true;
    }
    else
    {
        return false;
    }
}



Commands::SetRampRate::SetRampRate(double rampRate)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_RAMP_RATE_STR);
    CString rampRateStr;
    rampRateStr.Format("%.3f", rampRate);
    m_cmdPkt.AddData(rampRateStr);
}

BOOL Commands::SetRampRate::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//------------------------------------------------------------------

Commands::GetRampRate::GetRampRate()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_RAMP_RATE_STR);
}

BOOL Commands::GetRampRate::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 2)
    {
        m_rampRate = atof(rspPkt->GetData(1));
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------

Commands::GetTargetBlockTemp::GetTargetBlockTemp()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_BLOCK);
}

BOOL Commands::GetTargetBlockTemp::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 2 + CHANNEL_COUNT;
}

//------------------------------------------------------------------

Commands::GetTargetLidTemp::GetTargetLidTemp()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_LID);
}

BOOL Commands::GetTargetLidTemp::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 2 + LID_CHANNEL_COUNT;
}

//------------------------------------------------------------------

Commands::SetTargetBlockTemp::SetTargetBlockTemp(double temp)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    CString tempStr;
    tempStr.Format("%.2f", temp);
    m_cmdPkt.SetCmd(SET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_BLOCK);
    m_cmdPkt.AddData(tempStr);
}

Commands::SetTargetBlockTemp::SetTargetBlockTemp(CString& temp)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_BLOCK);
    m_cmdPkt.AddData(temp);
}

BOOL Commands::SetTargetBlockTemp::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//------------------------------------------------------------------

Commands::SetTargetLidTemp::SetTargetLidTemp(double temp)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    CString tempStr;
    tempStr.Format("%.2f", temp);
    m_cmdPkt.SetCmd(SET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_LID);
    m_cmdPkt.AddData(tempStr);
}

Commands::SetTargetLidTemp::SetTargetLidTemp(CString& temp)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_TARGET_TEMP_STR);
    m_cmdPkt.AddData(CHANNEL_LID);
    m_cmdPkt.AddData(temp);
}

BOOL Commands::SetTargetLidTemp::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//------------------------------------------------------------------

Commands::GetPWM::GetPWM()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_PWM_STR);
    m_cmdPkt.AddData(CHANNEL_ALL);
}

BOOL Commands::GetPWM::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 2 + CHANNEL_COUNT + LID_CHANNEL_COUNT)
    {
        for(int i = 0; i < CHANNEL_COUNT + LID_CHANNEL_COUNT; i++)
        {
            m_pwm[i] = rspPkt->GetData(2 + i);
        }
        return true;
    }
    else
    {
        return false;
    }
}


//------------------------------------------------------------------


Commands::EnableBlock::EnableBlock(BOOL enable)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(enable ? ENABLE_STR : DISABLE_STR);
    m_cmdPkt.AddData(CHANNEL_BLOCK);
}

BOOL Commands::EnableBlock::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}


//------------------------------------------------------------------


Commands::EnableLid::EnableLid(BOOL enable)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(enable ? ENABLE_STR : DISABLE_STR);
    m_cmdPkt.AddData(CHANNEL_LID);
}

BOOL Commands::EnableLid::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}


//------------------------------------------------------------------


Commands::EnableIllumination::EnableIllumination(InstrumentTypes::ExcitationColors color)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(ENABLE_ILLUMINATION_STR);

    switch(color)
    {
    default: // Fall through - handle as none
    case InstrumentTypes::EXCOLOR_NONE: break; // No Parameter
    case InstrumentTypes::EXCOLOR_470: m_cmdPkt.AddData("470"); break;
    case InstrumentTypes::EXCOLOR_530: m_cmdPkt.AddData("530"); break;
    case InstrumentTypes::EXCOLOR_586: m_cmdPkt.AddData("586"); break;
    case InstrumentTypes::EXCOLOR_630: m_cmdPkt.AddData("630"); break;
    }
}

BOOL Commands::EnableIllumination::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 1)
    {
        m_ledCurrent = 0;
        return TRUE;
    }
    
    if(rspPkt->GetDataLen() == 2)
    {
        m_ledCurrent = atoi(rspPkt->GetData(1));
        return TRUE;
    }

    return FALSE;
}


//------------------------------------------------------------------


Commands::SetIlluminationIntensity::SetIlluminationIntensity(InstrumentTypes::ExcitationColors color,
                                                             unsigned intensity)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    ITI_ASSERT_LESS_EQUAL(intensity, 100U);

    m_cmdPkt.SetCmd(SET_ILLUM_INTENSITY);

    switch(color)
    {
    case InstrumentTypes::EXCOLOR_470: m_cmdPkt.AddData("470"); break;
    case InstrumentTypes::EXCOLOR_530: m_cmdPkt.AddData("530"); break;
    case InstrumentTypes::EXCOLOR_586: m_cmdPkt.AddData("586"); break;
    case InstrumentTypes::EXCOLOR_630: m_cmdPkt.AddData("630"); break;
    case InstrumentTypes::EXCOLOR_NONE: break; // Fall through
    default: ITI_ERROR("Invalid Set Illumination Color");
    }

    m_cmdPkt.AddData("%u", intensity);
}

BOOL Commands::SetIlluminationIntensity::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}


//------------------------------------------------------------------


Commands::SetFilter::SetFilter(InstrumentTypes::EmissionColors filter)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_FILTER_STR);

    switch(filter)
    {
    case InstrumentTypes::EMCOLOR_510: m_cmdPkt.AddData("510"); break;
    case InstrumentTypes::EMCOLOR_555: m_cmdPkt.AddData("555"); break;
    case InstrumentTypes::EMCOLOR_620: m_cmdPkt.AddData("620"); break;
    case InstrumentTypes::EMCOLOR_640: m_cmdPkt.AddData("640"); break;
    case InstrumentTypes::EMCOLOR_665: m_cmdPkt.AddData("665"); break;
    case InstrumentTypes::EMCOLOR_710: m_cmdPkt.AddData("710"); break;
    }
}

BOOL Commands::SetFilter::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}





Commands::SetTempModel::SetTempModel(TempModel model)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_TEMP_MODEL_STR);

    switch(model)
    {
        case MODEL_SDA_OPEN:
            m_cmdPkt.AddData("sda_open");
            break;

        case MODEL_SDA_CLOSED:
            m_cmdPkt.AddData("sda_closed");
            break;

        case MODEL_PCR_OPEN:
            m_cmdPkt.AddData("pcr_open");
            break;

        case MODEL_PCR_CLOSED:
            m_cmdPkt.AddData("pcr_closed");
            break;

        default:
            ITI_ERROR("Unknown Model");
    }
}


BOOL Commands::SetTempModel::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}





Commands::SendFileData::SendFileData(unsigned fileIndex, const char* data, int dataLen)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(FILE_SEND_STR);

    m_cmdPkt.AddData("%u", fileIndex);

    CString hexData;
    for(int i = 0; i < dataLen; i++)
    {
        CString byte;
        byte.Format("%02X", ((unsigned)data[i]) & 0x000000ff);
        hexData += byte;
    }

    m_cmdPkt.AddData(hexData);
}



BOOL Commands::SendFileData::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}




Commands::UpgradeFirmware::UpgradeFirmware(unsigned fileSize, unsigned fileCrc)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(UPGRADE_FIRMWARE_STR);

    m_cmdPkt.AddData("%u", fileSize);
    m_cmdPkt.AddData("%u", fileCrc);
}



BOOL Commands::UpgradeFirmware::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}





Commands::UpgradeFPGA::UpgradeFPGA(unsigned fileSize, unsigned fileCrc)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(UPGRADE_FPGA_STR);

    m_cmdPkt.AddData("%u", fileSize);
    m_cmdPkt.AddData("%u", fileCrc);
}



BOOL Commands::UpgradeFPGA::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}





Commands::GetBlockSerialNumber::GetBlockSerialNumber()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_BLOCK_SERIAL_NUM_STR);
}



BOOL Commands::GetBlockSerialNumber::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() == 2)
    {
        m_serialNumber = rspPkt->GetData(1);
        return TRUE;
    }
    return FALSE;
}

//-----------------------------------------------------------------------------

Commands::Calibrate::Calibrate(const CString& channel, const CString& calPoint, double calTemp)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(CALIBRATE_STR);

    m_cmdPkt.AddData(channel);
    m_cmdPkt.AddData(calPoint);
    m_cmdPkt.AddData("%.3f", calTemp);
}

BOOL Commands::Calibrate::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::GetCal::GetCal(const CString& channelName)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
    , m_channelName(channelName)
{
    m_cmdPkt.SetCmd(GET_CAL_STR);
    m_cmdPkt.AddData(channelName);
}

BOOL Commands::GetCal::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 5)
        return FALSE;

    m_calLowPoint  = atof(rspPkt->GetData(1));
    m_calLow       = strtoul(rspPkt->GetData(2), NULL, 16);
    m_calHighPoint = atof(rspPkt->GetData(3));
    m_calHigh      = strtoul(rspPkt->GetData(4), NULL, 16);

    return TRUE;
}

//-----------------------------------------------------------------------------

Commands::SetCal::SetCal(const CString& channelName, unsigned calLow, unsigned calHigh)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_CAL_STR);
    m_cmdPkt.AddData(channelName);
    m_cmdPkt.AddData("0x%04X", calLow);
    m_cmdPkt.AddData("0x%04X", calHigh);
}

BOOL Commands::SetCal::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::SetCalPoint::SetCalPoint(const CString& channelName, BOOL high, double calPoint)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_CAL_POINT_STR);
    m_cmdPkt.AddData(channelName);
    m_cmdPkt.AddData("%s", high ? "high" : "low");
    m_cmdPkt.AddData("%.3f", calPoint);
}

BOOL Commands::SetCalPoint::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::SelfTest::SelfTest()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SELF_TEST_STR);
}



BOOL Commands::SelfTest::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::InitSystem::InitSystem()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(INIT_SYSTEM_STR);
}

BOOL Commands::InitSystem::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::OpenDoor::OpenDoor()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(OPEN_DOOR_STR);
}

BOOL Commands::OpenDoor::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//-----------------------------------------------------------------------------

Commands::CloseDoor::CloseDoor()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(CLOSE_DOOR_STR);
}

BOOL Commands::CloseDoor::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return false;
    }

    if(rspPkt->GetData(1) == DISPOSABLE_PRESENT_STR)
    {
        m_isDisposablePresent = true;
    }
    else if(rspPkt->GetData(1) == DISPOSABLE_NONE_STR ||
            rspPkt->GetData(1) == DISPOSABLE_UNKNOWN_STR)
    {
        m_isDisposablePresent = false;
    }
    else
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

Commands::GetStatus::GetStatus()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_FPGA_STATUS_STR);
}

BOOL Commands::GetStatus::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() < 2)
    {
        return FALSE;
    }

    if(rspPkt->GetData(1) == STATUS_IDLE_STR)
    {
        return TRUE;
    }

    for(int i = 1; i < rspPkt->GetDataLen(); i++)
    {
        m_pendingProcesses.Add(rspPkt->GetData(i));
    }

    return TRUE;
}

//----------------------------------------------------------------

Commands::GetDoorPosCalibration::GetDoorPosCalibration()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_MOTOR_CALBRATION);
    m_cmdPkt.AddData(DOOR_CLOSED);
}

BOOL Commands::GetDoorPosCalibration::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return FALSE;
    }

    m_position = atoi(rspPkt->GetData(1));

    return TRUE;
}

//----------------------------------------------------------------

Commands::SetDoorPosCalibration::SetDoorPosCalibration(unsigned position)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_MOTOR_CALBRATION);
    m_cmdPkt.AddData(DOOR_CLOSED);
    m_cmdPkt.AddData("%d", position);
}

BOOL Commands::SetDoorPosCalibration::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//----------------------------------------------------------------

Commands::GetLidPosCalibration::GetLidPosCalibration()
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(GET_MOTOR_CALBRATION);
    m_cmdPkt.AddData(LID_PCR);
}
BOOL Commands::GetLidPosCalibration::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return FALSE;
    }

    m_position = atoi(rspPkt->GetData(1));

    return TRUE;
}

//----------------------------------------------------------------

Commands::SetLidPosCalibration::SetLidPosCalibration(unsigned position)
    : Transaction(HEADER_TO_MASTER_FROM_HOST)
{
    m_cmdPkt.SetCmd(SET_MOTOR_CALBRATION);
    m_cmdPkt.AddData(LID_PCR);
    m_cmdPkt.AddData("%d", position);
}

BOOL Commands::SetLidPosCalibration::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//----------------------------------------------------------------

BOOL Commands::GetIlluminationCal::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return FALSE;
    }

    m_calibration = atoi(rspPkt->GetData(1));

    return TRUE;
};

//----------------------------------------------------------------

BOOL Commands::SetIlluminationCal::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//----------------------------------------------------------------

BOOL Commands::SetLowPowerIllumCal::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//----------------------------------------------------------------

BOOL Commands::GetLowPowerIllumCal::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return FALSE;
    }

    m_calibration = atoi(rspPkt->GetData(1));

    return TRUE;
};

//----------------------------------------------------------------

BOOL Commands::GetModelParams::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 4)
    {
        return FALSE;
    }

    m_w1 = atof(rspPkt->GetData(1));
    m_w2 = atof(rspPkt->GetData(2));
    m_w3 = atof(rspPkt->GetData(3));

    return TRUE;
};

//----------------------------------------------------------------

BOOL Commands::SetModelParam::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

//----------------------------------------------------------------

BOOL Commands::GetDebugCaptureSize::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 2)
    {
        return FALSE;
    }

    m_size = atoi(rspPkt->GetData(1));
    return true;
}

//----------------------------------------------------------------

BOOL Commands::GetDebugCapture::processRsp(CommPkt* rspPkt)
{
    if(rspPkt->GetDataLen() != 3)
    {
        return FALSE;
    }

    m_rawData = rspPkt->GetData(2);
    m_formattedDataLength = atoi(rspPkt->GetData(1));

    for(int i = 0; i < m_rawData.GetLength(); ++i)
    {
        char c = m_rawData[i];
        try
        {
            if(m_rawData[i] == '\\' && m_rawData.GetLength() > i + 3 && m_rawData[i + 1] == 'x')
            {
                char hexValue[3];
                hexValue[0] = m_rawData[i + 2];
                hexValue[1] = m_rawData[i + 3];
                hexValue[2] = 0;
                c = (char)stringToUnsigned(hexValue, 16);
                i += 3;
            }
        }
        catch(ParamException&)
        {
        }

        m_formattedData += c;
    }

    return TRUE;
}

//----------------------------------------------------------------

BOOL Commands::PauseDebugCapture::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}


//----------------------------------------------------------------

BOOL Commands::ResumeDebugCapture::processRsp(CommPkt* rspPkt)
{
    return rspPkt->GetDataLen() == 1;
}

Commands::MoveRelative::MoveRelative(std::string& motor, int steps)
	: Transaction(HEADER_TO_MASTER_FROM_HOST)
{
	m_cmdPkt.SetCmd(MOVE_RELATIVE);
	m_cmdPkt.AddData(motor.c_str());
	m_cmdPkt.AddData(boost::lexical_cast<std::string>(steps).c_str());
}

BOOL Commands::MoveRelative::processRsp(CommPkt* rspPkt)
{
	return rspPkt->GetDataLen() == 1;
}

BOOL Commands::ActuateArm::processRsp(CommPkt* rspPkt)
{
	return rspPkt->GetDataLen() == 1;
}

// EOF
