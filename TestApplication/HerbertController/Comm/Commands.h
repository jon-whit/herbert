/////////////////////////////////////////////////////////////
//
//  Commands.h
//
//  Plate Cycler Comm Protocol Commands
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef Commands_h
#define Commands_h

#include <CommPkt.h>
#include <InstrumentTypes.h>
#include <boost/shared_ptr.hpp>
#include <Exceptions.h>
#include <vector>


namespace Commands
{
    // Commands
    const char GET_PROTOCOL_VERSION[]     = "GetProtocolVersion";
    const char GET_FW_VERSION_STR[]       = "GetFirmwareVersion";
    const char GET_FPGA_VERSION_STR[]     = "GetFPGAVersion";
    const char GET_SERIAL_NUMBER_STR[]    = "GetSerialNumber";
    const char SET_SERIAL_NUMBER_STR[]    = "SetSerialNumber";
    const char RESET_STR[]                = "Reset";

    const char ABORT_STR[]                = "Abort";
    const char GET_CURRENT_TEMP_STR[]     = "ReadCurrentTemp";
    const char GET_TARGET_TEMP_STR[]      = "GetTargetTemp";
    const char SET_TARGET_TEMP_STR[]      = "SetTargetTemp";
    const char GET_PWM_STR[]              = "GetPWM";
    const char ENABLE_STR[]               = "EnableChannel";
    const char DISABLE_STR[]              = "DisableChannel";
    const char ENABLE_ILLUMINATION_STR[]  = "SetIllumination";
    const char SET_ILLUM_INTENSITY[]      = "SetIlluminationIntensity";
    const char SET_FILTER_STR[]           = "SetFilter";
    const char SET_TEMP_MODEL_STR[]       = "SetModel";
    const char FILE_SEND_STR[]            = "FileSend";
    const char UPGRADE_FIRMWARE_STR[]     = "Upgrade";
    const char UPGRADE_FPGA_STR[]         = "UpgradeFPGA";
    const char GET_BLOCK_SERIAL_NUM_STR[] = "GetBlockSerialNumber";
    const char CALIBRATE_STR[]            = "Calibrate";
    const char GET_CAL_STR[]              = "GetCal";
    const char SET_CAL_STR[]              = "SetCal";
    const char SET_CAL_POINT_STR[]        = "SetCalPoint";
    const char SELF_TEST_STR[]            = "SelfTest";
    const char INIT_SYSTEM_STR[]          = "InitSystem";
    const char OPEN_DOOR_STR[]            = "OpenDoor";
    const char CLOSE_DOOR_STR[]           = "CloseDoor";
    const char GET_FPGA_STATUS_STR[]      = "GetStatus";
    const char SET_RAMP_RATE_STR[]        = "SetRampRate";
    const char GET_RAMP_RATE_STR[]        = "GetRampRate";
	const char ACTUATE_ARM_STR[]		  = "ActuateArm";

	const char MOVE_RELATIVE[]			  = "MoveRelative";

    const char CHANNEL_ALL[]              = "ALL";
    const char CHANNEL_BLOCK[]            = "BLOCK";
    const char CHANNEL_LID[]              = "LID";
    const char CHANNEL_HEATSINK[]         = "HEATSINK";
    const char CHANNEL_AMBIENT[]          = "AMBIENT";

    const char CAL_HIGH_STR[]             = "high";
    const char CAL_LOW_STR[]              = "low";

    const char STATUS_IDLE_STR[]          = "Idle";

    const char GET_MOTOR_CALBRATION[]     = "GetMotorCalibration";
    const char SET_MOTOR_CALBRATION[]     = "SetMotorCalibration";
    const char DOOR_CLOSED[]              = "DOOR_CLOSED";
    const char LID_PCR[]                  = "LID_PCR";

    const char GET_ILLUMINATION_CAL[]     = "GetIlluminationCalibration";
    const char SET_ILLUMINATION_CAL[]     = "SetIlluminationCalibration";
    const char GET_LOW_POWER_ILLUM_CAL[]  = "GetIlluminationLowPowerCalibration";
    const char SET_LOW_POWER_ILLUM_CAL[]  = "SetIlluminationLowPowerCalibration";

    const char GET_MODEL_PARAMS[]         = "GetModelParams";
    const char SET_MODEL_PARAM[]          = "SetModelParam";

    const char GET_DEBUG_CAPTURE_SIZE[]   = "GetDebugCaptureSize";
    const char GET_DEBUG_CAPTURE[]        = "GetDebugCapture";
    const char PAUSE_DEBUG_CAPTURE[]      = "PauseDebugCapture";
    const char RESUME_DEBUG_CAPTURE[]     = "ResumeDebugCapture";

    const char PCB_DEVICE_ATX_STR[]       = "ATX";
    const char PCB_DEVICE_3_3V_STR[]      = "Power3.3V";
    const char PCB_DEVICE_12V_STR[]       = "Power12V";
    const char PCB_DEVICE_24V_STR[]       = "Power24V";
    const char PCB_DEVICE_LED_CTRL_STR[]  = "LED";
    const char PCB_DEVICE_SENSOR_STR[]    = "Sensor";
    const char PCB_DEVICE_LID_STR[]       = "Lid";
    const char PCB_DEVICE_FANS_STR[]      = "Fans";

    const char ERROR_MSG_KEY_STR[]        = "Error";
    const char LOG_MSG_KEY_STR[]          = "Log";

    const char DISPOSABLE_PRESENT_STR[]   = "Present";
    const char DISPOSABLE_NONE_STR[]      = "None";
    const char DISPOSABLE_UNKNOWN_STR[]   = "Unknown";

    enum ModelParam
    {
        ModelParam_20h,
        ModelParam_20c,
        ModelParam_50h,
        ModelParam_50c,

        ModelParam_Count
    };
    const char* const MODEL_PARAMS[] = { "25h", "25c", "50h", "50c" };
    const char* ConvertModelParam(ModelParam modelParam);
    ModelParam ConvertModelParam(const std::string& modelParam);



    enum WParam
    {
        WParam_w1,
        WParam_w2,
        WParam_w3,

        WParam_Count
    };
    const char* const W_PARAMS[] = { "w1", "w2", "w3" };
    const char* ConvertWParam(WParam wParam);
    WParam ConvertWParam(const std::string& wParam);




    class Transaction
    {
    public:
        class Exception : public ITI::Exception
        {
        public:
            Exception(const std::string& ex) : ITI::Exception(ex) {}
        };

        enum Type
        {
            TypeManual,
            TypeAbort,
            TypeGetFpgaFirmwareVersion,
            TypeGetPcbFirmwareVersion,
            TypeGetFpgaVersion,
            TypeGetFpgaSerialNumber,
            TypeGetPcbSerialNumber,
            TypeGetTemperatureBlock,
            TypeGetTemperatureLid,
            TypeGetTemperatureHeatsink,
            TypeGetTemperatureAmbient,
            TypeGetTemperatureAll,
            TypeSetRampRate,
            TypeGetRampRate,
            TypeGetTargetBlockTemp,
            TypeSetTargetBlockTemp,
            TypeGetTargetLidTemp,
            TypeSetTargetLidTemp,
            TypeGetPWM,
            TypeEnableBlock,
            TypeEnableLid,
            TypeEnableIllumination,
            TypeSetIlluminationIntensity,
            TypeSetFilter,
            TypeSetTempModel,
            TypeSendFileData,
            TypeUpgrade,
            TypeGetBlockSerialNumber,
            TypeCalibrate,
            TypeGetCal,
            TypeSetCal,
            TypeSetCalPoint,
            TypeSelfTest,
            TypeInitSystem,
            TypeOpenDoor,
            TypeCloseDoor,
            TypeGetStatus,
            TypeGetPcbStatus,
            TypeGet3_3VSupplyVoltage,
            TypeGet12VSupplyVoltage,
            TypeGet24VSupplyVoltage,
            TypeGetSensorVoltage,
            TypeGetATXSupplyOnState,
            TypeGet24VSupplyOnState,
            TypeGetLEDCtrlOnState,
            TypeGetSensorOnState,
            TypeGetLidOnState,
            TypeGetFansOnState,
            TypeSetATXSupplyOnState,
            TypeSet24VSupplyOnState,
            TypeSetLEDCtrlOnState,
            TypeSetSensorOnState,
            TypeSetLidOnState,
            TypeSetFansOnState,
            TypeGetDoorPosCalibration,
            TypeSetDoorPosCalibration,
            TypeGetLidPosCalibration,
            TypeSetLidPosCalibration,
            TypeGetIlluminationCalibration,
            TypeSetIlluminationCalibration,
            TypeGetLowPowerIlluminationCalibration,
            TypeSetLowPowerIlluminationCalibration,
            TypeGetModelParams,
            TypeSetModelParam,
            TypeGetDebugCaptureSize,
            TypeGetDebugCapture,
            TypePauseDebugCapture,
            TypeResumeDebugCapture,
			TypeMoveRelative,
			TypeActuateArm,
        };

        typedef boost::shared_ptr<Transaction> shared_ptr;

    public:
        Transaction(const CString& Header, BOOL setHeader = TRUE);
        virtual ~Transaction() {}
        virtual Type type() = 0;                      //Pure virtual so Transaction can't be created
        BOOL validateRsp(CommPkt* rspPkt);

        CString cmdPktString()      {if(m_cmdPktString.IsEmpty()) m_cmdPktString = m_cmdPkt.GetFullPktStr(); return m_cmdPktString;}
        CString rspPktString()      {return m_rspString;}
        BOOL    validRsp()          {return m_validRsp;}
        CString cmd()               {return m_cmdPkt.GetCmd();}
        CString sequenceNum()       {return m_cmdPkt.GetSeqNum();}
        CString status()            {return m_statusStr;}
        BOOL    statusOk()          {return m_statusStr == COMM_STATUS_OK;}
        BOOL    isOfflineRsp()      {return m_isOfflineRsp;}


        int     getErrorCode()      {return m_errorCode;}
        CString getErrorString()    {return m_errorString;}

        void    setStatus(const CString& status)      { m_statusStr = status; }
        void    setErrorString(const CString& error)  { m_errorString = error; }
        void    setOfflineRspFlag()                   { m_isOfflineRsp = TRUE; }

        void    setResonse(const CString& statusStr, int errorCode, const CString& errorString);

        static BOOL isErrorMessage(CommPkt* rspPkt);
        static BOOL isLogMessage(CommPkt* rspPkt);

    private:
        // Explicitly disallow...
        Transaction(const Transaction&);
        Transaction& operator=(const Transaction&);

    protected:
        virtual BOOL processRsp(CommPkt* rspPkt) = 0; //Pure virtual so Transaction can't be created

        CommPkt m_cmdPkt;
        CString m_cmdPktString;
        BOOL    m_validRsp;
        CString m_statusStr;
        CString m_rspString;
        int     m_errorCode;
        CString m_errorString;
        BOOL    m_isOfflineRsp;
    };


    class PendingResponse
    {
    public:
        typedef boost::shared_ptr<PendingResponse> shared_ptr;

        PendingResponse(Commands::Transaction::shared_ptr cmdTransaction,
                        CommPkt::shared_ptr               pendingRsp)
            : m_cmdTransaction(cmdTransaction), m_pendingRsp(pendingRsp) {}

        Commands::Transaction::shared_ptr cmd()        { return m_cmdTransaction; }
        CommPkt::shared_ptr               pendingRsp() { return m_pendingRsp; }

    private:
        PendingResponse();
        PendingResponse(const PendingResponse&);
        PendingResponse& operator=(const PendingResponse&);

        Commands::Transaction::shared_ptr m_cmdTransaction;
        CommPkt::shared_ptr               m_pendingRsp;
    };



    class InfoResponse
    {
    public:
        typedef boost::shared_ptr<InfoResponse> shared_ptr;

        InfoResponse(Commands::Transaction::shared_ptr cmdTransaction,
                     CommPkt::shared_ptr               infoRsp)
            : m_cmdTransaction(cmdTransaction), m_infoRsp(infoRsp) {}

        Commands::Transaction::shared_ptr cmd()     { return m_cmdTransaction; }
        CommPkt::shared_ptr               infoRsp() { return m_infoRsp; }

    private:
        InfoResponse();
        InfoResponse(const InfoResponse&);
        InfoResponse& operator=(const InfoResponse&);

        Commands::Transaction::shared_ptr m_cmdTransaction;
        CommPkt::shared_ptr               m_infoRsp;
    };



    class ErrorMessage
    {
    public:
        typedef boost::shared_ptr<ErrorMessage> shared_ptr;

        ErrorMessage(CommPkt::shared_ptr pkt) : m_pkt(pkt){}

        CommPkt::shared_ptr pkt()    { return m_pkt; }
        int                 number() { return m_pkt->GetErrorCode(); }
        CString             string() { return m_pkt->GetErrorString(); }

    private:
        ErrorMessage();
        ErrorMessage(const ErrorMessage&);
        ErrorMessage& operator=(const ErrorMessage&);

        CommPkt::shared_ptr m_pkt;
    };



    class LogMessage
    {
    public:
        typedef boost::shared_ptr<LogMessage> shared_ptr;

        LogMessage(CommPkt::shared_ptr pkt) : m_pkt(pkt)
        {
            for(int i = 1; i < pkt->GetDataLen(); ++i)
            {
                if(!m_string.IsEmpty())
                {
                    m_string += " ";
                }

                m_string += pkt->GetData(i);
            }
        }

        CommPkt::shared_ptr pkt()    { return m_pkt; }
        CString             string() { return m_string; }

    private:
        LogMessage();
        LogMessage(const LogMessage&);
        LogMessage& operator=(const LogMessage&);

        CommPkt::shared_ptr m_pkt;
        CString             m_string;
    };



    class Manual : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeManual;}
        unsigned rspParamCount() {return m_rspParams.size();}
        const CString& rspParam(unsigned index) {return m_rspParams[index];}
    protected:
        Manual(const CString& header, const CString& cmd, BOOL addHeader = FALSE);
        std::vector<CString> m_rspParams;
    };



    class FpgaManual : public Manual
    {
    public:
        FpgaManual(const CString& cmd, BOOL addHeader = FALSE);
    };



    class PcbManual : public Manual
    {
    public:
        PcbManual(const CString& cmd, BOOL addHeader = FALSE);
    };



    class Abort : public Transaction
    {
    public:
        Abort();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeAbort;}
    };



    //----------------------------------------------------------------------------



    class GetFirmwareVersion : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);

        const CString& firmwareVersion() { return m_firmwareVersion; }
        const CString& buildDate()       { return m_buildDate; }
    protected:
        GetFirmwareVersion(const CString& header);

        CString m_firmwareVersion;
        CString m_buildDate;
    };



    class GetFpgaFirmwareVersion : public GetFirmwareVersion
    {
    public:
        GetFpgaFirmwareVersion();
        Type type() {return TypeGetFpgaFirmwareVersion;}
    };



    class GetPcbFirmwareVersion : public GetFirmwareVersion
    {
    public:
        GetPcbFirmwareVersion();
        Type type() {return TypeGetPcbFirmwareVersion;}
    };



    //----------------------------------------------------------------------------



    class GetFpgaVersion : public Transaction
    {
    public:
        GetFpgaVersion();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetFpgaVersion;}
        const CString& fpgaVersion() { return m_fpgaVersion; }
    private:
        CString m_fpgaVersion;
    };



    //----------------------------------------------------------------------------



    class GetSerialNumber: public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        const CString& serialNumber() { return m_serialNumber; }
    protected:
        GetSerialNumber(const CString& header);
        CString m_serialNumber;
    };



    class GetFpgaSerialNumber : public GetSerialNumber
    {
    public:
        GetFpgaSerialNumber();
        Type type() {return TypeGetFpgaSerialNumber;}
    };

    class GetPcbSerialNumber : public GetSerialNumber
    {
    public:
        GetPcbSerialNumber();
        Type type() {return TypeGetPcbSerialNumber;}
    };



    //----------------------------------------------------------------------------



    class GetTemperatureBlock : public Transaction
    {
    public:
        GetTemperatureBlock();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTemperatureBlock;}

        const CString& tempStr(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT); return m_temp[index]; }
        double         tempVal(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT); return atof(m_temp[index]); }
    
    private:
        CString m_temp[CHANNEL_COUNT];
    };




    class GetTemperatureLid : public Transaction
    {
    public:
        GetTemperatureLid();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTemperatureLid;}

        const CString& temp() { return m_temp; }
    
    private:
        CString m_temp;
    };



    class GetTemperatureHeatsink : public Transaction
    {
    public:
        GetTemperatureHeatsink();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTemperatureHeatsink;}

        const CString& temp() { return m_temp; }
    
    private:
        CString m_temp;
    };



    class GetTemperatureAmbient : public Transaction
    {
    public:
        GetTemperatureAmbient();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTemperatureAmbient;}

        const CString& temp() { return m_temp; }
    
    private:
        CString m_temp;
    };



    class GetTemperatureAll : public Transaction
    {
    public:
        GetTemperatureAll();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTemperatureAll;}

        const CString& blockTempStr(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT); return m_blockTemps[index]; }
        const CString& lidCenterTempStr()      { return m_lidCenterTemp; }
        const CString& lidRingTempStr()        { return m_lidRingTemp; }
        const CString& ambientTempStr()        { return m_ambientTemp; }
        const CString& heatsinkTempStr()       { return m_heatsinkTemp; }
        double         blockTempVal(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT); return atof(m_blockTemps[index]); }
        double         lidCenterTempVal()      { return atof(m_lidCenterTemp); }
        double         lidRingTempVal()        { return atof(m_lidRingTemp); }
        double         ambientTempVal()        { return atof(m_ambientTemp); }
        double         heatsinkTempVal()       { return atof(m_heatsinkTemp); }
    
    private:
        CString m_blockTemps[CHANNEL_COUNT];
        CString m_lidCenterTemp;
        CString m_lidRingTemp;
        CString m_ambientTemp;
        CString m_heatsinkTemp;
    };




    class SetRampRate : public Transaction
    {
    public:
        SetRampRate(double rampRate);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetRampRate;}
    };



    class GetRampRate : public Transaction
    {
    public:
        GetRampRate();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetRampRate;}

        double rampRate() {return m_rampRate;}

    private:
        double m_rampRate;
    };



    class GetTargetBlockTemp : public Transaction
    {
    public:
        GetTargetBlockTemp();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTargetBlockTemp;}
    };



    class GetTargetLidTemp : public Transaction
    {
    public:
        GetTargetLidTemp();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetTargetLidTemp;}
    };



    class SetTargetBlockTemp : public Transaction
    {
    public:
        SetTargetBlockTemp(double temp);
        SetTargetBlockTemp(CString& temp);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetTargetBlockTemp;}
    };



    class SetTargetLidTemp : public Transaction
    {
    public:
        SetTargetLidTemp(double temp);
        SetTargetLidTemp(CString& temp);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetTargetLidTemp;}
    };



    class GetPWM : public Transaction
    {
    public:
        GetPWM();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetPWM;}

        const CString& pwmStr(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT + LID_CHANNEL_COUNT); return m_pwm[index]; }
        double         pwmVal(int index) { ITI_ASSERT(index >= 0 && index < CHANNEL_COUNT + LID_CHANNEL_COUNT); return atof(m_pwm[index]); }
    
    private:
        CString m_pwm[CHANNEL_COUNT + LID_CHANNEL_COUNT];
    };




    class EnableBlock : public Transaction
    {
    public:
        EnableBlock(BOOL enable = TRUE);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeEnableBlock;}
    };



    class EnableLid : public Transaction
    {
    public:
        EnableLid(BOOL enable = TRUE);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeEnableLid;}
    };


    class EnableIllumination : public Transaction
    {
    public:
        EnableIllumination(InstrumentTypes::ExcitationColors color);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeEnableIllumination;}
        unsigned GetCurrent() {return m_ledCurrent;}
    private:
        unsigned m_ledCurrent;
    };

    
    class SetIlluminationIntensity : public Transaction
    {
    public:
        SetIlluminationIntensity(InstrumentTypes::ExcitationColors color, unsigned intensity);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetIlluminationIntensity;}
    };

    class SetFilter : public Transaction
    {
    public:
        SetFilter(InstrumentTypes::EmissionColors filter);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetFilter;}
    };



    class SetTempModel : public Transaction
    {
    public:
        enum TempModel
        {
            MODEL_SDA_OPEN,
            MODEL_SDA_CLOSED,
            MODEL_PCR_OPEN,
            MODEL_PCR_CLOSED,
        };
        SetTempModel(TempModel model);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetTempModel;}
    };


    class SendFileData : public Transaction
    {
    public:
        SendFileData(unsigned fileIndex, const char* data, int dataLen);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSendFileData;}
    };



    class UpgradeFirmware : public Transaction
    {
    public:
        UpgradeFirmware(unsigned fileSize, unsigned fileCrc);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeUpgrade;}
    };



    class UpgradeFPGA : public Transaction
    {
    public:
        UpgradeFPGA(unsigned fileSize, unsigned fileCrc);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeUpgrade;}
    };



    class GetBlockSerialNumber : public Transaction
    {
    public:
        GetBlockSerialNumber();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetBlockSerialNumber;}
        CString blockSerialNumber() {return m_serialNumber;}
    private:
        CString m_serialNumber;
    };



    class Calibrate : public Transaction
    {
    public:
        Calibrate(const CString& channel, const CString& calPoint, double calTemp);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeCalibrate;}
    };



    class GetCal : public Transaction
    {
    public:
        GetCal(const CString& channelName);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetCal;}

        CString  channelName()  {return m_channelName;}
        unsigned calLow()       {return m_calLow;}
        unsigned calHigh()      {return m_calHigh;}
        double   calLowPoint()  {return m_calLowPoint;}
        double   calHighPoint() {return m_calHighPoint;}
    private:
        CString  m_channelName;
        unsigned m_calLow;
        unsigned m_calHigh;
        double   m_calLowPoint;
        double   m_calHighPoint;
    };


    class SetCal : public Transaction
    {
    public:
        SetCal(const CString& channelName, unsigned calLow, unsigned calHigh);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetCal;}
    };



    class SetCalPoint : public Transaction
    {
    public:
        SetCalPoint(const CString& channelName, BOOL high, double calPoint);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetCalPoint;}
    };



    class SelfTest : public Transaction
    {
    public:
        SelfTest();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSelfTest;}
    };



    class InitSystem : public Transaction
    {
    public:
        InitSystem();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeInitSystem;}
    };



    class OpenDoor : public Transaction
    {
    public:
        OpenDoor();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeOpenDoor;}
    };



    class CloseDoor : public Transaction
    {
    public:
        CloseDoor();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeCloseDoor;}
        bool isDisposablePresent() {return m_isDisposablePresent;}
    private:
        bool m_isDisposablePresent;
    };



    class GetStatus : public Transaction
    {
    public:
        GetStatus();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetStatus;}

        unsigned pendingProcessCount() {return m_pendingProcesses.GetCount();}
        CString  getPendingProcess(unsigned index) {return m_pendingProcesses[index];}

    private:
        CStringArray m_pendingProcesses;
    };



    class GetPcbStatus : public Transaction
    {
    public:
        GetPcbStatus();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetPcbStatus;}
        BOOL PowerSupply3_3VError() { return m_powerSupply3_3VError; }
        BOOL PowerSupply12VError()  { return m_powerSupply12VError; }
        BOOL PowerSupply24VError()  { return m_powerSupply24VError; }
        BOOL SensorPowerError()     { return m_sensorPowerError; }
        BOOL PwrDownRequested()     { return m_pwrDownRequested; }

        CString PowerSupply3_3VVoltage() { return m_powerSupply3_3VVoltage; }
        CString PowerSupply12VVoltage()  { return m_powerSupply12VVoltage; }
        CString PowerSupply24VVoltage()  { return m_powerSupply24VVoltage; }
        CString SensorPowerVoltage()     { return m_sensorPowerVoltage; }
    private:
        BOOL m_powerSupply3_3VError;
        BOOL m_powerSupply12VError;
        BOOL m_powerSupply24VError;
        BOOL m_sensorPowerError;
        BOOL m_pwrDownRequested;

        CString m_powerSupply3_3VVoltage;
        CString m_sensorPowerVoltage;
        CString m_powerSupply12VVoltage;
        CString m_powerSupply24VVoltage;
    };


    class GetVoltage : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        CString Voltage() {return m_voltage;}
    protected:
        GetVoltage(const CString& device);
        CString m_voltage;
    };

    #define GET_DEVICE_VOLTAGE(className, deviceStr, transactionType) \
    class className : public GetVoltage                               \
    {                                                                 \
    public:                                                           \
        className() : GetVoltage(deviceStr) {}                        \
        Type type() {return transactionType;}                         \
    };

    GET_DEVICE_VOLTAGE(Get3_3VSupplyVoltage, PCB_DEVICE_3_3V_STR,       TypeGet3_3VSupplyVoltage);
    GET_DEVICE_VOLTAGE(Get12VSupplyVoltage,  PCB_DEVICE_12V_STR,        TypeGet12VSupplyVoltage);
    GET_DEVICE_VOLTAGE(Get24VSupplyVoltage,  PCB_DEVICE_24V_STR,        TypeGet24VSupplyVoltage);
    GET_DEVICE_VOLTAGE(GetSensorVoltage,     PCB_DEVICE_SENSOR_STR,     TypeGetSensorVoltage);







    class GetOnState : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        BOOL On() {return m_on;}
    protected:
        GetOnState(const CString& device);
        BOOL m_on;
    };

    #define GET_DEVICE_ON_STATE(className, deviceStr, transactionType) \
    class className : public GetOnState                                \
    {                                                                  \
    public:                                                            \
        className() : GetOnState(deviceStr) {}                         \
        Type type() {return transactionType;}                          \
    };

    GET_DEVICE_ON_STATE(GetATXSupplyOnState,  PCB_DEVICE_ATX_STR,        TypeGetATXSupplyOnState);
    GET_DEVICE_ON_STATE(Get24VSupplyOnState,  PCB_DEVICE_24V_STR,        TypeGet24VSupplyOnState);
    GET_DEVICE_ON_STATE(GetLEDCtrlOnState,    PCB_DEVICE_LED_CTRL_STR,   TypeGetLEDCtrlOnState);
    GET_DEVICE_ON_STATE(GetSensorOnState,     PCB_DEVICE_SENSOR_STR,     TypeGetSensorOnState);
    GET_DEVICE_ON_STATE(GetLidOnState,        PCB_DEVICE_LID_STR,        TypeGetLidOnState);
    GET_DEVICE_ON_STATE(GetFansOnState,       PCB_DEVICE_FANS_STR,       TypeGetFansOnState);





    class SetOnState : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
    protected:
        SetOnState(const CString& device, BOOL on);
        BOOL m_on;
    };

    #define SET_DEVICE_ON_STATE(className, deviceStr, transactionType) \
    class className : public SetOnState                                \
    {                                                                  \
    public:                                                            \
        className(BOOL on) : SetOnState(deviceStr, on) {}              \
        Type type() {return transactionType;}                          \
    };

    SET_DEVICE_ON_STATE(SetATXSupplyOnState,  PCB_DEVICE_ATX_STR,        TypeSetATXSupplyOnState);
    SET_DEVICE_ON_STATE(Set24VSupplyOnState,  PCB_DEVICE_24V_STR,        TypeSet24VSupplyOnState);
    SET_DEVICE_ON_STATE(SetLEDCtrlOnState,    PCB_DEVICE_LED_CTRL_STR,   TypeSetLEDCtrlOnState);
    SET_DEVICE_ON_STATE(SetSensorOnState,     PCB_DEVICE_SENSOR_STR,     TypeSetSensorOnState);
    SET_DEVICE_ON_STATE(SetLidOnState,        PCB_DEVICE_LID_STR,        TypeSetLidOnState);
    SET_DEVICE_ON_STATE(SetFansOnState,       PCB_DEVICE_FANS_STR,       TypeSetFansOnState);



    class GetDoorPosCalibration: public Transaction
    {
    public:
        GetDoorPosCalibration();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetDoorPosCalibration;}
        unsigned Position() {return m_position;}
    protected:
        unsigned m_position;
    };

    class SetDoorPosCalibration: public Transaction
    {
    public:
        SetDoorPosCalibration(unsigned position);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetDoorPosCalibration;}
    };

    class GetLidPosCalibration: public Transaction
    {
    public:
        GetLidPosCalibration();
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetLidPosCalibration;}
        unsigned Position() {return m_position;}
    protected:
        unsigned m_position;
    };

    class SetLidPosCalibration: public Transaction
    {
    public:
        SetLidPosCalibration(unsigned position);
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetLidPosCalibration;}
    };

    //----------------------------------------------------------------------------

    class GetIlluminationCal : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetIlluminationCalibration;}

        GetIlluminationCal(InstrumentTypes::ExcitationColors color) :
            Transaction(HEADER_TO_MASTER_FROM_HOST),
            m_color(color)
        {
            m_cmdPkt.SetCmd(GET_ILLUMINATION_CAL);
            m_cmdPkt.AddData(InstrumentTypes::getExcitationColorName(m_color).c_str());
        }

        InstrumentTypes::ExcitationColors getColor()       {return m_color;}
        unsigned                          getCalibration() {return m_calibration;}
    protected:
        InstrumentTypes::ExcitationColors m_color;
        unsigned                          m_calibration;
    private:
        GetIlluminationCal();
    };

    //----------------------------------------------------------------------------

    class SetIlluminationCal : public Transaction
    {
    public:
        SetIlluminationCal(InstrumentTypes::ExcitationColors color,
                           unsigned                          cal) :
            Transaction(HEADER_TO_MASTER_FROM_HOST),
            m_color(color), m_cal(cal)
        {
            m_cmdPkt.SetCmd(SET_ILLUMINATION_CAL);
            m_cmdPkt.AddData(InstrumentTypes::getExcitationColorName(color).c_str());
            m_cmdPkt.AddData("%u", cal);
        }

        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetIlluminationCalibration;}
    protected:
        InstrumentTypes::ExcitationColors m_color;
        unsigned                          m_cal; 
    private:
        SetIlluminationCal();
    };

    //----------------------------------------------------------------------------

    class GetLowPowerIllumCal : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetLowPowerIlluminationCalibration;}

        GetLowPowerIllumCal(InstrumentTypes::ExcitationColors color) :
            Transaction(HEADER_TO_MASTER_FROM_HOST),
            m_color(color)
        {
            m_cmdPkt.SetCmd(GET_LOW_POWER_ILLUM_CAL);
            m_cmdPkt.AddData(InstrumentTypes::getExcitationColorName(m_color).c_str());
        }

        InstrumentTypes::ExcitationColors getColor()       {return m_color;}
        unsigned                          getCalibration() {return m_calibration;}
    protected:
        InstrumentTypes::ExcitationColors m_color;
        unsigned                          m_calibration;
    private:
        GetLowPowerIllumCal();
    };

    //----------------------------------------------------------------------------

    class SetLowPowerIllumCal : public Transaction
    {
    public:
        SetLowPowerIllumCal(InstrumentTypes::ExcitationColors color,
                            unsigned                          cal) :
            Transaction(HEADER_TO_MASTER_FROM_HOST),
            m_color(color), m_cal(cal)
        {
            m_cmdPkt.SetCmd(SET_LOW_POWER_ILLUM_CAL);
            m_cmdPkt.AddData(InstrumentTypes::getExcitationColorName(color).c_str());
            m_cmdPkt.AddData("%u", cal);
        }
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetLowPowerIlluminationCalibration;}
    protected:
        InstrumentTypes::ExcitationColors m_color;
        unsigned                          m_cal; 
    private:
        SetLowPowerIllumCal();
    };

    //----------------------------------------------------------------------------

    class GetModelParams : public Transaction
    {
    public:
        GetModelParams(const std::string& channelName, ModelParam model):
             Transaction(HEADER_TO_MASTER_FROM_HOST),
             m_channelName(channelName), m_modelParam(model)
        {
            m_cmdPkt.SetCmd(GET_MODEL_PARAMS);
            m_cmdPkt.AddData(channelName.c_str());
            m_cmdPkt.AddData(MODEL_PARAMS[model]);
        }

        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetModelParams;}

        std::string GetChannelName()      {return m_channelName;}
        ModelParam  GetModelParam()       {return m_modelParam;}
        std::string GetModelParamString() {return MODEL_PARAMS[m_modelParam];}
        double      GetW1()               {return m_w1;}
        double      GetW2()               {return m_w2;}
        double      GetW3()               {return m_w3;}

    protected:
        std::string m_channelName;
        ModelParam  m_modelParam;
        double      m_w1;
        double      m_w2;
        double      m_w3;
    };

    #define GET_MODEL_PARAMS(className, model)                     \
    class className : public GetModelParams                        \
    {                                                              \
    public:                                                        \
        className(const std::string& channelName) :                \
            GetModelParams(channelName, model) {}                  \
    };

    GET_MODEL_PARAMS(Get25hModelParams, ModelParam_20h);
    GET_MODEL_PARAMS(Get25cModelParams, ModelParam_20c);
    GET_MODEL_PARAMS(Get50hModelParams, ModelParam_50h);
    GET_MODEL_PARAMS(Get50cModelParams, ModelParam_50c);



    //----------------------------------------------------------------------------


    class SetModelParam : public Transaction
    {
    public:
        SetModelParam(const std::string& channelName, ModelParam modelParam, WParam wParam, double wValue):
             Transaction(HEADER_TO_MASTER_FROM_HOST),
             m_channelName(channelName), m_modelParam(modelParam),
             m_wParam(wParam), m_wValue(wValue)
        {
            formatPkt();
        }

        SetModelParam(const std::string& channelName, const std::string& modelParam, const std::string& wParam, double wValue):
             Transaction(HEADER_TO_MASTER_FROM_HOST),
             m_channelName(channelName), m_modelParam(ConvertModelParam(modelParam)),
             m_wParam(ConvertWParam(wParam)), m_wValue(wValue)
        {
            formatPkt();
        }

        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeSetModelParam;}

        std::string GetChannelName() {return m_channelName;}
        ModelParam  GetModelParam()  {return m_modelParam;}
        WParam      GetWParam()      {return m_wParam;}
        double      GetWValue()      {return m_wValue;}

    protected:
        void formatPkt()
        {
            m_cmdPkt.SetCmd(SET_MODEL_PARAM);
            m_cmdPkt.AddData(m_channelName.c_str());
            m_cmdPkt.AddData(ConvertModelParam(m_modelParam));
            m_cmdPkt.AddData(ConvertWParam(m_wParam));
            m_cmdPkt.AddData("%.03f", m_wValue);
        }

        std::string m_channelName;
        ModelParam  m_modelParam;
        WParam      m_wParam;
        double      m_wValue;
    };

    #define SET_MODEL_PARAMS(className, model)                     \
    class className : public SetModelParam                         \
    {                                                              \
    public:                                                        \
        className(const std::string& channelName,                  \
                  WParam wParam, double wValue) :                  \
            SetModelParam(channelName, model, wParam, wValue) {}   \
    };

    SET_MODEL_PARAMS(Set25hModelParams, ModelParam_20h);
    SET_MODEL_PARAMS(Set25cModelParams, ModelParam_20c);
    SET_MODEL_PARAMS(Set50hModelParams, ModelParam_50h);
    SET_MODEL_PARAMS(Set50cModelParams, ModelParam_50c);

    //----------------------------------------------------------------------------

    class GetDebugCaptureSize : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetDebugCaptureSize;}

        GetDebugCaptureSize() :
            Transaction(HEADER_TO_MASTER_FROM_HOST)
        {
            m_cmdPkt.SetCmd(GET_DEBUG_CAPTURE_SIZE);
        }

        unsigned getSize() {return m_size;}
    protected:
        unsigned m_size;
    };

    //----------------------------------------------------------------------------

    class GetDebugCapture : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeGetDebugCapture;}

        GetDebugCapture(unsigned offset, unsigned size) :
            Transaction(HEADER_TO_MASTER_FROM_HOST)
        {
            m_cmdPkt.SetCmd(GET_DEBUG_CAPTURE);
            m_cmdPkt.AddData("%u", offset);
            m_cmdPkt.AddData("%u", size);
        }

        CString  getRawData()             {return m_rawData;}
        CString  getFormattedData()       {return m_formattedData;}
        unsigned getFormattedDataLength() {return m_formattedDataLength;}
    protected:
        CString  m_rawData;
        CString  m_formattedData;
        unsigned m_formattedDataLength;
    private:
        GetDebugCapture();
    };

    //----------------------------------------------------------------------------

    class PauseDebugCapture : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypePauseDebugCapture;}

        PauseDebugCapture() :
            Transaction(HEADER_TO_MASTER_FROM_HOST)
        {
            m_cmdPkt.SetCmd(PAUSE_DEBUG_CAPTURE);
        }
    };

	//----------------------------------------------------------------------------
	
	class ActuateArm : public Transaction
	{
	public:
		BOOL processRsp(CommPkt* rspPkt);
		Type type() {return TypeActuateArm;}

		ActuateArm(const CString& arm) :
			Transaction(HEADER_TO_MASTER_FROM_HOST)
		{
			m_cmdPkt.SetCmd(ACTUATE_ARM_STR);
			m_cmdPkt.AddData(arm);
		}

		protected:
			CString m_arm;
	};
    //----------------------------------------------------------------------------

    class ResumeDebugCapture : public Transaction
    {
    public:
        BOOL processRsp(CommPkt* rspPkt);
        Type type() {return TypeResumeDebugCapture;}

        ResumeDebugCapture() :
            Transaction(HEADER_TO_MASTER_FROM_HOST)
        {
            m_cmdPkt.SetCmd(RESUME_DEBUG_CAPTURE);
        }
    };

	class MoveRelative : public Transaction 
	{
	public:
		MoveRelative(std::string& motor, int steps);
		BOOL processRsp(CommPkt* rspPkt);
		Type type() {return TypeMoveRelative;}
	};
}

#endif
