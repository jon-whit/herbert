/////////////////////////////////////////////////////////////
//
//  comm.c
//
//  Communication Protocol Processor
//
//  Available Commands
//
//  Abort
//  DisableCommWatchdog
//  DisableMotors
//  ExecuteMove[Move]
//  FileSend[FileSize][FileData]
//  GetActiveState
//  GetFirmwareVersion
//  GetFPGAVersion
//  GetStatus
//  GoToIdle
//  InitSystem
//  MoveRelative[Motor][Steps]
//  Reboot
//  Reset
//  SendErrorMessage<msg..>
//  SendLogMessage<msg..>
//  TestGetSensors
//  TestGetSwitch
//  Upgrade[FileSize][FileCRC]
//  UpgradeFPGA[FileSize][FileCRC]

#include <comm.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <crc16.h>

#include <timer.h>
#include <serial.h>
#include <update.h>
#include <version.h>
#include <motion.h>
#include <steppers.h>
#include <system.h>
#include <reboot.h>
#include <startup.h>
#include <stepper_hw.h>
#include <switch.h>

///////////////////////////////////////////////////
// Options

#define DISPLAY_CMD_PKTS     0
#define DISPLAY_RX_TX        0

//Define out the static keyword to stop the link errors
//we are getting.
#define static

///////////////////////////////////////////////////
// Constants

enum CommDelimiters
{
    paramDelimiter = ' ',
};

#define MIN_RSP_PARAM_COUNT           4

#define VAR_PARAM_COUNT               (-1)

#define HOST_INTERFACE_PORT           SBC_SERIAL_PORT

#define COMM_PROCESS_HOST_PERIOD_ms   10
#define COMM_PROCESS_SLAVE_PERIOD_ms  10
#define COMM_CONNECTION_TIMEOUT_s     600

#define COMM_BYTE_TIME_us             87

#define COMM_CMD_BUF_SIZE             MAX_RX_DATA_SIZE
#define COMM_RSP_BUF_SIZE             MAX_TX_DATA_SIZE
#define MAX_PARAMS_COUNT              30

#define TX_TIMEOUT_us                 10000

#define TO_MASTER_FROM_HOST_KEY       "MH"
#define TO_HOST_FROM_MASTER_KEY       "HM"

#define LOG_MSG_KEY                   "Log"
#define ERROR_MSG_KEY                 "Error"

#define U_MOTOR_KEY                   "U"
#define F_MOTOR_KEY                   "F"
#define R_MOTOR_KEY                   "R"
// #define D_MOTOR_KEY                   "D"
// #define B_MOTOR_KEY                   "B"
// #define L_MOTOR_KEY                   "L"

#define FAST_STEPPER_KEY              "FAST"
#define SLOW_STEPPER_KEY              "SLOW"
#define RAMP_STEPS_STEPPER_KEY        "RAMP_STEPS"

#define RSP_OK                        "OK"
#define RSP_ERROR                     "ER"
#define RSP_PENDING                   "PE"
#define RSP_INFORMATION_PKT           "IP"

#define OFFLINE_COMMAND_SEQ_NUM_BUF_SIZE 100
#define OFFLINE_COMMAND_CMD_BUF_SIZE     100

enum OfflineCommands
{
    OfflineCommand_executingMove,
    OfflineCommand_InitializeSystem,

    OfflineCommand_count,
};

const char* OfflineCommandNames[] =
{
    "ExecuteMove",
    "InitializingSystem",
};

///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    unsigned bufByteCount;
    char     buf[COMM_CMD_BUF_SIZE];
    char*    dstSrc;
    char*    seqNum;
    char*    cmd;
    unsigned paramCount;
    char*    params[MAX_PARAMS_COUNT];
    char*    crc;
} CmdPkt;

typedef struct
{
    unsigned bufByteCount;
    unsigned paramCount;
    char*    bufPtr;
    char     buf[COMM_RSP_BUF_SIZE];
    char*    seqNum;
    char*    cmd;
    char*    status;
    char*    payload;
} RspPkt;

typedef struct
{
    char* cmd;
    int   paramCount;
    void  (*cmdHandler)(CmdPkt* cmdPkt);
} CommCommand;

typedef struct
{
    bool active;
    char seqNum[OFFLINE_COMMAND_SEQ_NUM_BUF_SIZE];
    char cmd[OFFLINE_COMMAND_CMD_BUF_SIZE];
} PendingCommand;

typedef struct
{
    Timer          serialInterfaceTimer;
    Signal         serialInterfaceRxSignal;

    bool           monitoringConnection;
    Timer          connectionTimer;

    PendingCommand pendingCommands[OfflineCommand_count];

    bool           commWatchdogDisabled;

    int            moveCount;
} CommData;

///////////////////////////////////////////////////
// Local function prototypes

// Callback functions
static void signalOfflineTaskCompleteCallback(int offlineTask, ErrorCodes error, const char* errorDesc);

static bool parseCmdPkt(CmdPkt* cmdPkt);
static void processCmdPkt(CmdPkt* cmdPkt);

// Not static to prevent "brlid" bug in EDK 10.1 linker.
/*static*/ void addParamDelimiter(RspPkt* rspPkt);
/*static*/ void initRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, const char* status);
/*static*/ void addParamToRspPkt(RspPkt* rspPkt, const char *format, ...);
/*static*/ void addStringParamToRspPkt(RspPkt* rspPkt, const char* str);
/*static*/ void sendRspPkt(RspPkt* rspPkt);

/*static*/ bool checkPendingCmds(CmdPkt* cmdPkt, uint32 offlineCmdIndex, uint32 ignoreMask);
static void registerPendingCmd(uint32 offlineCmdIndex, CmdPkt* cmdPkt);
static void sendOfflineRspOk(uint32 offlineCmdIndex, const char* extraParams);
static void sendOfflineRspError(uint32 offlineCmdIndex, ErrorCodes errorCode, const char* errorDesc);
static void sendOfflineRspAborted(uint32 offlineCmdIndex);

static void initErrorRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, ErrorCodes errorCode);
static void initIpPkt(RspPkt* rspPkt, const char* cmd);
static bool initOfflineRsp(RspPkt* rspPkt, uint32 offlineCmdIndex, bool clear);
static void initIpErrorPkt(RspPkt* rspPkt, int errorCode);
static void initIpLogPkt(RspPkt* rspPkt);


// Status functions
/*static*/ void sendRspOk(CmdPkt* cmdPkt);
static void registerAndSendRspPending(uint32 offlineCmdIndex, CmdPkt* cmdPkt);
static void sendRspPending(CmdPkt* cmdPkt);
/*static*/ void sendRspError(CmdPkt* cmdPkt, ErrorCodes errorCode, const char* errorDesc);

static void sendRspStatusIntOutOfRange(CmdPkt* cmdPkt, int min, int max, int value);
static void sendRspStatusHexOutOfRange(CmdPkt* cmdPkt, uint32 min, uint32 max, uint32 value);
static void sendRspStatusInvalidParameter(CmdPkt* cmdPkt);
static void sendRspStatusInvalidParameterCount(CmdPkt* cmdPkt, unsigned minExpectedParamCount, unsigned maxExpectedParamCount);
static void sendRspStatusUnknownCommand(CmdPkt* cmdPkt);
static void sendRspStatusSystemBusy(CmdPkt* cmdPkt, const char* errorDesc);
static void sendRspStatusInvalidImage(CmdPkt* cmdPkt, const char* errorDesc);
static void sendRspStatusMissingFileData(CmdPkt* cmdPkt);
static void sendRspStatusInvalidFileData(CmdPkt* cmdPkt);


// String parsing functions
static bool getIntValue(const char* str, int* value, int base);
static bool getUnsignedValue(const char* str, uint32* value, int base);

static bool validateIntValue(CmdPkt* cmdPkt, const char* str, int* value);
static bool validateUnsignedValue(CmdPkt* cmdPkt, const char* str, uint32* value);
static bool validateHexValue(CmdPkt* cmdPkt, const char* str, uint32* value);

static bool validateIntParameterRange(CmdPkt* cmdPkt, int paramIndex, int minValue, int maxValue, int* value);
static bool validateMotorParameter(CmdPkt* cmdPkt, int paramIndex, StepperMotor *motor);

// Handler helpers
static void systemAbort();

// Command Handlers

static void chAbort(CmdPkt* cmdPkt);
static void chDisableCommWatchdog(CmdPkt* cmdPkt);
static void chDisableMotors(CmdPkt* cmdPkt);
static void chExecuteMove(CmdPkt* cmdPkt);
static void chFileSend(CmdPkt* cmdPkt);
static void chGetActiveState(CmdPkt* cmdPkt);
static void chGetFirmwareVersion(CmdPkt* cmdPkt);
static void chGetFPGAVersion(CmdPkt* cmdPkt);
static void chGetStatus(CmdPkt* cmdPkt);
static void chGoToIdle(CmdPkt* cmdPkt);
static void chInitSystem(CmdPkt* cmdPkt);
static void chMoveRelative(CmdPkt* cmdPkt);
static void chReboot(CmdPkt* cmdPkt);
static void chReset(CmdPkt* cmdPkt);
static void chSendErrorMessage(CmdPkt* cmdPkt);
static void chSendLogMessage(CmdPkt* cmdPkt);
static void chTestGetSensors(CmdPkt* cmdPkt);
static void chTestGetSwitch(CmdPkt* cmdPkt);
static void chUpgradeFirmware(CmdPkt* cmdPkt);
static void chUpgradeFPGA(CmdPkt* cmdPkt);

///////////////////////////////////////////////////
// Local data

static const CommCommand commCommands[] =
{   // Cmd,                  Param Count,     Cmd Handler,
    { "Abort",               0,               chAbort,               },
    { "DisableCommWatchdog", 0,               chDisableCommWatchdog, },
    { "DisableMotors",       0,               chDisableMotors,       },
    { "ExecuteMove",         1,               chExecuteMove,         },
    { "FileSend",            2,               chFileSend,            },
    { "GetActiveState",      0,               chGetActiveState,      },
    { "GetFirmwareVersion",  0,               chGetFirmwareVersion,  },
    { "GetFPGAVersion",      0,               chGetFPGAVersion,      },
    { "GetStatus",           0,               chGetStatus,           },
    { "GoToIdle",            0,               chGoToIdle,            },
    { "InitSystem",          0,               chInitSystem,          },
    { "MoveRelative",        2,               chMoveRelative,        },
    { "Reboot",              0,               chReboot,              },
    { "Reset",               0,               chReset,               },
    { "SendErrorMessage",    VAR_PARAM_COUNT, chSendErrorMessage,    },
    { "SendLogMessage",      VAR_PARAM_COUNT, chSendLogMessage,      },
    { "TestGetSensors",      0,               chTestGetSensors,      },
    { "TestGetSwitch",       0,               chTestGetSwitch,       },
    { "Upgrade",             2,               chUpgradeFirmware,     },
    { "UpgradeFPGA",         2,               chUpgradeFPGA,         },
    // End of table marker
    { NULL, 0, NULL }
};

static CommData commData;

///////////////////////////////////////////////////
// Interface functions

void commInit()
{
    memset(&commData, 0, sizeof(commData));

    commData.monitoringConnection = false;
    commData.commWatchdogDisabled = false;

    commData.moveCount = 0;

    setSerialPortRxSignal(HOST_INTERFACE_PORT, &commData.serialInterfaceRxSignal);
    startTimer(&commData.serialInterfaceTimer, MSEC_TO_TICKS(COMM_PROCESS_HOST_PERIOD_ms));
    startTimer(&commData.connectionTimer, SEC_TO_TICKS(COMM_CONNECTION_TIMEOUT_s));
}

bool commProcess(void* unused)
{
    CmdPkt cmdPkt;

    if(timerExpired(&commData.serialInterfaceTimer))
    {
        if(!commData.commWatchdogDisabled && commData.monitoringConnection && timerExpired(&commData.connectionTimer))
        {
            systemAbort();
            sendErrorMsg(err_connectionTimeout, "FPGA did not receive communication from host for more than "
                                                STRINGIFY(COMM_CONNECTION_TIMEOUT_s)
                                                " seconds - All operations aborted");
        }

        cmdPkt.bufByteCount = serialGetPkt(HOST_INTERFACE_PORT, (uint8*)cmdPkt.buf, COMM_CMD_BUF_SIZE - 1);

        if(cmdPkt.bufByteCount)
        {
            cmdPkt.buf[cmdPkt.bufByteCount] = 0;

            #if DISPLAY_RX_TX
                printf("RX: %s\n", cmdPkt.buf);
            #endif

            if(parseCmdPkt(&cmdPkt))
            {
                commData.monitoringConnection = true;
                startTimer(&commData.connectionTimer, SEC_TO_TICKS(COMM_CONNECTION_TIMEOUT_s));
                processCmdPkt(&cmdPkt);
            }
        }

        startTimer(&commData.serialInterfaceTimer, MSEC_TO_TICKS(COMM_PROCESS_HOST_PERIOD_ms));
    }

    return true;
}

void sendErrorMsg(ErrorCodes errorCode, const char* errorDesc)
{
    ASSERT(errorCode < num_errors);

    RspPkt rspPkt;

    initIpErrorPkt(&rspPkt, errorCode);

    if(errorDesc)
    {
        addStringParamToRspPkt(&rspPkt, "-");
        addStringParamToRspPkt(&rspPkt, errorDesc);
    }

    sendRspPkt(&rspPkt);
}

void sendLogMsg(const char* logMsg)
{
    ASSERT(logMsg);

    RspPkt rspPkt;

    initIpLogPkt(&rspPkt);
    addStringParamToRspPkt(&rspPkt, logMsg);
    printf("Log: %s\n", logMsg);

    sendRspPkt(&rspPkt);
}

///////////////////////////////////////////////////
// Local functions

static void signalOfflineTaskCompleteCallback(int offlineTask, ErrorCodes error, const char* errorDesc)
{
    ASSERT(offlineTask >= 0 && offlineTask < OfflineCommand_count);
    ASSERT(error >= 0 && error < num_errors);
    commData.moveCount++;

    if(offlineTask == OfflineCommand_executingMove && commData.moveCount == 2)
    {
        
    }

    if(error == err_noError)
    {
        sendOfflineRspOk(offlineTask, errorDesc);
    }
    else
    {
        sendOfflineRspError(offlineTask, error, errorDesc);
    }
}

static bool parseCmdPkt(CmdPkt* cmdPkt)
{
    ASSERT(cmdPkt);
    ASSERT(cmdPkt->bufByteCount < sizeof(cmdPkt->buf) - 1);

    cmdPkt->dstSrc     = NULL;
    cmdPkt->seqNum     = NULL;
    cmdPkt->cmd        = NULL;
    cmdPkt->crc        = NULL;
    cmdPkt->paramCount = 0;

    if(cmdPkt->buf[0] == paramDelimiter || !isprint(cmdPkt->buf[0]))
    {
        // First character is bad
        return false;
    }

    // Find and verify CRC
    char* crcParam = strrchr(cmdPkt->buf, paramDelimiter);

    if(!crcParam)
    {
        // No CRC parameter found
        return false;
    }

    *crcParam++ = 0;
    cmdPkt->crc = crcParam;

    uint32 calcCrc = calcCRC16(INITIAL_CRC16_VALUE, cmdPkt->buf, strlen(cmdPkt->buf));
    uint32 pktCrc;

    if(!getUnsignedValue(cmdPkt->crc, &pktCrc, 10) ||
       (pktCrc != 0 && calcCrc != pktCrc))
    {
        // CRC parameter not correct
        printf("Invalid Cmd Pkt CRC (%lu): '%s %s' \n", calcCrc, cmdPkt->buf, cmdPkt->crc);
        return false;
    }

    unsigned i;
    bool     startOfParam = true;
    char*    bufPtr;

    for(i = 0, bufPtr = cmdPkt->buf;
        *bufPtr && i < cmdPkt->bufByteCount;
        i++, bufPtr++)
    {
        if(!isprint(*bufPtr))
        {
            // Invalid character
            return false;
        }

        if(*bufPtr == paramDelimiter)
        {
            //Null terminate parameter
            *bufPtr = 0;

            startOfParam = true;
        }
        else
        {
            if(startOfParam)
            {
                startOfParam = false;

                if(!cmdPkt->dstSrc)
                {
                    cmdPkt->dstSrc = bufPtr;
                }
                else if(!cmdPkt->seqNum)
                {
                    cmdPkt->seqNum = bufPtr;
                }
                else if(!cmdPkt->cmd)
                {
                    cmdPkt->cmd = bufPtr;
                }
                else
                {
                    if(cmdPkt->paramCount >= MAX_PARAMS_COUNT)
                    {
                        // Too many parameters
                        return false;
                    }

                    cmdPkt->params[cmdPkt->paramCount++] = bufPtr;
                }
            }
        }
    }

    //Null terminate last parameter
    *bufPtr = 0;

    if(!cmdPkt->dstSrc || strcmp(cmdPkt->dstSrc, TO_MASTER_FROM_HOST_KEY) != 0 ||
       !cmdPkt->seqNum || !cmdPkt->cmd)
    {
        printf("Cmd Pkt header invalid\n");
        return false;
    }

    #if DISPLAY_CMD_PKTS
        printf("Cmd Pkt:\n");
        printf("  Dst/Src   = %s\n", cmdPkt->dstSrc);
        printf("  Seq Num   = %s\n", cmdPkt->seqNum);
        printf("  Cmd       = %s\n", cmdPkt->cmd);
        printf("  Param Cnt = %d\n", cmdPkt->paramCount);
        printf("  Params:\n");
        for(i = 0; i < cmdPkt->paramCount; i++)
        {
            printf("    Param %d = %s\n", i, cmdPkt->params[i]);
        }
        printf("  CRC       = %s\n", cmdPkt->crc);
    #endif

    return true;
}

static void processCmdPkt(CmdPkt* cmdPkt)
{
    const CommCommand* commCommandIterator = commCommands;

    while(commCommandIterator->cmd)
    {
        if(strcmp(cmdPkt->cmd, commCommandIterator->cmd) == 0)
        {
            if((int)cmdPkt->paramCount == commCommandIterator->paramCount ||
               commCommandIterator->paramCount == VAR_PARAM_COUNT)
            {
                commCommandIterator->cmdHandler(cmdPkt);
            }
            else
            {
                sendRspStatusInvalidParameterCount(cmdPkt, commCommandIterator->paramCount, commCommandIterator->paramCount);
            }

            return;
        }

        commCommandIterator++;
    }

    sendRspStatusUnknownCommand(cmdPkt);
}

void addParamDelimiter(RspPkt* rspPkt)
{
    if(rspPkt->paramCount > 0 && rspPkt->bufByteCount < COMM_RSP_BUF_SIZE - 1)
    {
        *rspPkt->bufPtr++ = paramDelimiter;
        rspPkt->bufByteCount++;
    }
}

void addParamToRspPkt(RspPkt* rspPkt, const char *format, ...)
{
    addParamDelimiter(rspPkt);

    int     count;
    va_list argp;

    va_start (argp, format);

    count = vsnprintf(rspPkt->bufPtr, COMM_RSP_BUF_SIZE - rspPkt->bufByteCount, format, argp);
    va_end (argp);

    rspPkt->bufPtr    += count;
    rspPkt->bufByteCount += count;

    ASSERT(rspPkt->bufByteCount <= COMM_RSP_BUF_SIZE);

    rspPkt->paramCount++;
}

void addStringParamToRspPkt(RspPkt* rspPkt, const char* str)
{
    addParamDelimiter(rspPkt);

    while(*str && rspPkt->bufByteCount < COMM_RSP_BUF_SIZE - 1)
    {
        *rspPkt->bufPtr++ = *str++;
        rspPkt->bufByteCount++;
    }

    *rspPkt->bufPtr = '\0';

    rspPkt->paramCount++;
}

void sendRspPkt(RspPkt* rspPkt)
{
    uint16 crc = calcCRC16(INITIAL_CRC16_VALUE, rspPkt->buf, rspPkt->bufByteCount);

    addParamToRspPkt(rspPkt, "%u", crc);

    #if DISPLAY_RX_TX
        printf("TX: %s\n", rspPkt->buf);
    #endif

    if(!strncmp(rspPkt->status, RSP_ERROR, 2))
    {
        printf("Error: %s\n", rspPkt->cmd);
    }

    if(!serialSendPkt(HOST_INTERFACE_PORT, (uint8*)rspPkt->buf, rspPkt->bufByteCount, TX_TIMEOUT_us))
    {
        printf("TX packet send timeout: %s\n", rspPkt->buf);
    }
}

bool checkPendingCmds(CmdPkt* cmdPkt,  uint32 offlineCmdIndex, uint32 ignoreMask)
{
    ASSERT(offlineCmdIndex < OfflineCommand_count);


    // Check for matching offline cmd already pending
    if(commData.pendingCommands[offlineCmdIndex].active &&
       strcmp(commData.pendingCommands[offlineCmdIndex].seqNum, cmdPkt->seqNum) == 0)
    {
        // Resend pending rsp
        sendRspPending(cmdPkt);
        return false;
    }


    // Check for any pending cmds (except ignoreMask)
    int i;
    for(i = 0; i < OfflineCommand_count; i++)
    {
        if(commData.pendingCommands[i].active && (!(ignoreMask & (1 << i))))
        {
            sendRspStatusSystemBusy(cmdPkt, OfflineCommandNames[i]);
            return false;
        }
    }

    // Okay to continue
    return true;
}

static void registerPendingCmd(uint32 offlineCmdIndex, CmdPkt* cmdPkt)
{
    ASSERT(offlineCmdIndex < OfflineCommand_count);
    ASSERT(!commData.pendingCommands[offlineCmdIndex].active);
    ASSERT(cmdPkt);

    commData.pendingCommands[offlineCmdIndex].active = true;

    strncpy(commData.pendingCommands[offlineCmdIndex].seqNum,
            cmdPkt->seqNum, sizeof(commData.pendingCommands[offlineCmdIndex].seqNum));
    commData.pendingCommands[offlineCmdIndex].
        seqNum[sizeof(commData.pendingCommands[offlineCmdIndex].seqNum)-1] = '\0';

    strncpy(commData.pendingCommands[offlineCmdIndex].cmd,
            cmdPkt->cmd, sizeof(commData.pendingCommands[offlineCmdIndex].cmd));
    commData.pendingCommands[offlineCmdIndex].
        cmd[sizeof(commData.pendingCommands[offlineCmdIndex].cmd)-1] = '\0';
}

static void sendOfflineRspOk(uint32 offlineCmdIndex, const char* extraParams)
{
    RspPkt rspPkt;

    if(initOfflineRsp(&rspPkt, offlineCmdIndex, true))
    {
        addStringParamToRspPkt(&rspPkt, RSP_OK);
        if(extraParams)
        {
            addStringParamToRspPkt(&rspPkt, extraParams);
        }
        sendRspPkt(&rspPkt);
    }
}

static void sendOfflineRspError(uint32 offlineCmdIndex, ErrorCodes errorCode, const char* errorDesc)
{
    ASSERT(errorCode < num_errors);

    RspPkt rspPkt;

    if(initOfflineRsp(&rspPkt, offlineCmdIndex, true))
    {
        addStringParamToRspPkt(&rspPkt, RSP_ERROR);
        addParamToRspPkt(&rspPkt, "%03d", errorCode);
        addStringParamToRspPkt(&rspPkt, error_strings[errorCode]);

        if(errorDesc && strlen(errorDesc) > 0)
        {
            addStringParamToRspPkt(&rspPkt, "-");
            addStringParamToRspPkt(&rspPkt, errorDesc);
        }

        sendRspPkt(&rspPkt);
    }
}

static void sendOfflineRspAborted(uint32 offlineCmdIndex)
{
    ASSERT(offlineCmdIndex < OfflineCommand_count);

    RspPkt rspPkt;

    if(initOfflineRsp(&rspPkt, offlineCmdIndex, true))
    {
        addStringParamToRspPkt(&rspPkt, RSP_ERROR);
        addParamToRspPkt(&rspPkt, "%03d", err_operationAborted);
        addStringParamToRspPkt(&rspPkt, error_strings[err_operationAborted]);
        sendRspPkt(&rspPkt);
    }
}

void initRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, const char* status)
{
    ASSERT(rspPkt);
    ASSERT(cmdPkt);
    ASSERT(status);

    rspPkt->bufByteCount = 0;
    rspPkt->paramCount   = 0;
    rspPkt->bufPtr       = rspPkt->buf;

    addStringParamToRspPkt(rspPkt, TO_HOST_FROM_MASTER_KEY);
    rspPkt->seqNum = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, cmdPkt->seqNum);
    rspPkt->cmd = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, cmdPkt->cmd);
    rspPkt->status = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, status);
    rspPkt->payload = rspPkt->bufPtr + 1;
}

static void initErrorRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, ErrorCodes errorCode)
{
    ASSERT(rspPkt);
    ASSERT(cmdPkt);
    ASSERT(errorCode < num_errors);

    initRspPkt(rspPkt, cmdPkt, RSP_ERROR);
    addParamToRspPkt(rspPkt, "%03d", errorCode);
    addStringParamToRspPkt(rspPkt, error_strings[errorCode]);
}

static bool initOfflineRsp(RspPkt* rspPkt, uint32 offlineCmdIndex, bool clear)
{
    ASSERT(offlineCmdIndex < OfflineCommand_count);
    ASSERT(rspPkt);

    if(commData.pendingCommands[offlineCmdIndex].active)
    {
        if(clear)
        {
            commData.pendingCommands[offlineCmdIndex].active = false;
        }

        rspPkt->bufByteCount = 0;
        rspPkt->paramCount   = 0;
        rspPkt->bufPtr       = rspPkt->buf;

        addStringParamToRspPkt(rspPkt, TO_HOST_FROM_MASTER_KEY);
        rspPkt->seqNum = rspPkt->bufPtr + 1;
        addStringParamToRspPkt(rspPkt, commData.pendingCommands[offlineCmdIndex].seqNum);
        rspPkt->cmd = rspPkt->bufPtr + 1;
        addStringParamToRspPkt(rspPkt, commData.pendingCommands[offlineCmdIndex].cmd);
        rspPkt->status = rspPkt->bufPtr + 1;

        return true;
    }

    return false;
}

static void initIpPkt(RspPkt* rspPkt, const char* cmd)
{
    ASSERT(rspPkt);
    ASSERT(cmd);

    rspPkt->bufByteCount = 0;
    rspPkt->paramCount   = 0;
    rspPkt->bufPtr       = rspPkt->buf;

    addStringParamToRspPkt(rspPkt, TO_HOST_FROM_MASTER_KEY);
    rspPkt->seqNum = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, "00");
    rspPkt->cmd = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, cmd);
    rspPkt->status = rspPkt->bufPtr + 1;
    addStringParamToRspPkt(rspPkt, RSP_INFORMATION_PKT);
    rspPkt->payload = rspPkt->bufPtr + 1;
}

static void initIpErrorPkt(RspPkt* rspPkt, int errorCode)
{
    initIpPkt(rspPkt, ERROR_MSG_KEY);
    addParamToRspPkt(rspPkt, "%03d", errorCode);
    addStringParamToRspPkt(rspPkt, error_strings[errorCode]);
}

static void initIpLogPkt(RspPkt* rspPkt)
{
    initIpPkt(rspPkt, LOG_MSG_KEY);
}

void sendRspOk(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    sendRspPkt(&rspPkt);
}

static void registerAndSendRspPending(uint32 offlineCmdIndex, CmdPkt* cmdPkt)
{
    registerPendingCmd(offlineCmdIndex, cmdPkt);
    sendRspPending(cmdPkt);
}

static void sendRspPending(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_PENDING);
    sendRspPkt(&rspPkt);
}

void sendRspError(CmdPkt* cmdPkt, ErrorCodes errorCode, const char* errorDesc)
{
    ASSERT(errorCode < num_errors);

    RspPkt rspPkt;

    initErrorRspPkt(&rspPkt, cmdPkt, errorCode);

    if(errorDesc)
    {
        addStringParamToRspPkt(&rspPkt, "-");
        addStringParamToRspPkt(&rspPkt, errorDesc);
    }

    sendRspPkt(&rspPkt);
}

static void sendRspStatusIntOutOfRange(CmdPkt* cmdPkt, int min, int max, int value)
{
    RspPkt rspPkt;

    initErrorRspPkt(&rspPkt, cmdPkt, err_parameterOutOfRange);

    addParamToRspPkt(&rspPkt, "%d to %d, value %d", min, max, value);

    sendRspPkt(&rspPkt);
}

static void sendRspStatusHexOutOfRange(CmdPkt* cmdPkt, uint32 min, uint32 max, uint32 value)
{
    RspPkt rspPkt;

    initErrorRspPkt(&rspPkt, cmdPkt, err_parameterOutOfRange);

    addParamToRspPkt(&rspPkt, "0x%X to 0x%X, value 0x%X", min, max, value);

    sendRspPkt(&rspPkt);
}

static void sendRspStatusInvalidParameter(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidParameter, NULL);
}

static void sendRspStatusInvalidParameterCount(CmdPkt* cmdPkt, unsigned minExpectedParamCount, unsigned maxExpectedParamCount)
{
    RspPkt rspPkt;

    initErrorRspPkt(&rspPkt, cmdPkt, err_invalidParameterCount);

    if(minExpectedParamCount == maxExpectedParamCount)
    {
        addParamToRspPkt(&rspPkt, "- Expected %d parameter%s, received %d",
                         maxExpectedParamCount, maxExpectedParamCount == 1 ? "" : "s", cmdPkt->paramCount);
    }
    else
    {
        addParamToRspPkt(&rspPkt, "- Expected %d to %d parameter%s, received %d",
                         minExpectedParamCount, maxExpectedParamCount,
                         maxExpectedParamCount == 1 ? "" : "s", cmdPkt->paramCount);
    }

    if(cmdPkt->paramCount > 0)
    {
        addStringParamToRspPkt(&rspPkt, "-");
    }

    int paramIndex;
    for(paramIndex = 0; paramIndex < (int)cmdPkt->paramCount; ++paramIndex)
    {
        addStringParamToRspPkt(&rspPkt, cmdPkt->params[paramIndex]);
    }

    sendRspPkt(&rspPkt);
}

static void sendRspStatusUnknownCommand(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_unknownCommand, NULL);
}

static void sendRspStatusSystemBusy(CmdPkt* cmdPkt, const char* errorDesc)
{
    sendRspError(cmdPkt, err_systemBusy, errorDesc);
}

static void sendRspStatusInvalidImage(CmdPkt* cmdPkt, const char* errorDesc)
{
    sendRspError(cmdPkt, err_invalidUpgradeImage, errorDesc);
}

static void sendRspStatusMissingFileData(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_missingFileData, NULL);
}

static void sendRspStatusInvalidFileData(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidFileData, NULL);
}

static bool getIntValue(const char* str, int* value, int base)
{
    char* endPtr;

    *value = strtol(str, &endPtr, base);

    return !(*endPtr);
}

static bool getUnsignedValue(const char* str, uint32* value, int base)
{
    char* endPtr;

    *value = strtoul(str, &endPtr, base);

    return !(*endPtr);
}

static bool validateIntValue(CmdPkt* cmdPkt, const char* str, int* value)
{
    if(!getIntValue(str, value, 10))
    {
        sendRspStatusInvalidParameter(cmdPkt);
        return false;
    }

    return true;
}

static bool validateUnsignedValue(CmdPkt* cmdPkt, const char* str, uint32* value)
{
    if(!getUnsignedValue(str, value, 10))
    {
        sendRspStatusInvalidParameter(cmdPkt);
        return false;
    }

    return true;
}

static bool validateHexValue(CmdPkt* cmdPkt, const char* str, uint32* value)
{
    if(!getUnsignedValue(str, value, 16))
    {
        sendRspStatusInvalidParameter(cmdPkt);
        return false;
    }

    return true;
}

static bool validateIntParameterRange(CmdPkt* cmdPkt, int paramIndex, int minValue, int maxValue, int* value)
{
    ASSERT(cmdPkt);
    ASSERT(value);
    ASSERT(paramIndex < (int)cmdPkt->paramCount && paramIndex >= 0);

    if(!(validateIntValue(cmdPkt, cmdPkt->params[paramIndex], value)))
    {
        return false;
    }

    if(*value < minValue || *value > maxValue)
    {
        sendRspStatusIntOutOfRange(cmdPkt, minValue, maxValue, *value);
        return false;
    }

    return true;
}

static bool validateMotorParameter(CmdPkt* cmdPkt, int paramIndex, StepperMotor *motor)
{
    if(strcmp(U_MOTOR_KEY, cmdPkt->params[paramIndex]) == 0)
    {
        *motor = stepperU;
    }
    else if(strcmp(F_MOTOR_KEY, cmdPkt->params[paramIndex]) == 0)
    {
        *motor = stepperF;
    }
    else if(strcmp(R_MOTOR_KEY, cmdPkt->params[paramIndex]) == 0)
    {
        *motor = stepperR;
    }
    else
    {
        sendRspStatusInvalidParameter(cmdPkt);
        return false;
    }
    return true;
}

static void systemAbort()
{
    commData.monitoringConnection = false;

    motionAbort();
    disableMotors();

    int i;
    for(i = 0; i < OfflineCommand_count; i++)
    {
        sendOfflineRspAborted(i);
    }

    printf("System Abort\n");
}


///////////////////////////////////////////////////
// Command handler functions

static void chAbort(CmdPkt* cmdPkt)
{
    sendRspOk(cmdPkt);
    systemAbort();
}

static void chDisableCommWatchdog(CmdPkt* cmdPkt)
{
    commData.commWatchdogDisabled = true;
    sendRspOk(cmdPkt);
}

static void chDisableMotors(CmdPkt* cmdPkt)
{
    disableMotors();
    sendRspOk(cmdPkt);
}

static void chExecuteMove(CmdPkt* cmdPkt)
{
    if(!checkPendingCmds(cmdPkt, OfflineCommand_executingMove, 0)) return;

    registerAndSendRspPending(OfflineCommand_executingMove, cmdPkt);

    printf("Executing Command %s\n", cmdPkt->params[0]);
    if      (strcmp("R",  cmdPkt->params[0]) == 0) {executeMove(move_RClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Rb", cmdPkt->params[0]) == 0) {executeMove(move_RCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("R2", cmdPkt->params[0]) == 0) {executeMove(move_R2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("F",  cmdPkt->params[0]) == 0) {executeMove(move_FClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Fb", cmdPkt->params[0]) == 0) {executeMove(move_FCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("F2", cmdPkt->params[0]) == 0) {executeMove(move_F2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("U",  cmdPkt->params[0]) == 0) {executeMove(move_UClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Ub", cmdPkt->params[0]) == 0) {executeMove(move_UCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("U2", cmdPkt->params[0]) == 0) {executeMove(move_U2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("L",  cmdPkt->params[0]) == 0) {executeMove(move_LClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Lb", cmdPkt->params[0]) == 0) {executeMove(move_LCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("L2", cmdPkt->params[0]) == 0) {executeMove(move_L2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("B",  cmdPkt->params[0]) == 0) {executeMove(move_BClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Bb", cmdPkt->params[0]) == 0) {executeMove(move_BCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("B2", cmdPkt->params[0]) == 0) {executeMove(move_B2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("D",  cmdPkt->params[0]) == 0) {executeMove(move_DClockwise,        signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("Db", cmdPkt->params[0]) == 0) {executeMove(move_DCounterClockwise, signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else if (strcmp("D2", cmdPkt->params[0]) == 0) {executeMove(move_D2,                signalOfflineTaskCompleteCallback, OfflineCommand_executingMove);} 
    else
    {
        printf("Invalid Move\n");
    }
}

static void chFileSend(CmdPkt* cmdPkt)
{
    int fileOffset;

    if(!validateIntParameterRange(cmdPkt, 0, 0, MAX_FILE_SIZE, &fileOffset)) return;


    char* fileData = cmdPkt->params[1];

    ASSERT(fileData);

    while(*fileData)
    {
        int   i;
        uint8 byte = 0;

        for(i = 0; i < 2; i++)
        {
            if(!*fileData)
            {
                sendRspStatusMissingFileData(cmdPkt);
                return;
            }

            if((*fileData < '0' || *fileData > '9') &&
               (*fileData < 'A' || *fileData > 'F'))
            {
                sendRspStatusInvalidFileData(cmdPkt);
                return;
            }

            byte <<= 4;
            byte += (*fileData >= 'A') ? (*fileData - 'A' + 10) : (*fileData - '0');

            fileData++;
        }


        if(!storeFileData(fileOffset, byte))
        {
            sendRspStatusMissingFileData(cmdPkt);
            return;
        }

        fileOffset++;
    }

    sendRspOk(cmdPkt);
}

static void chGetActiveState(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;
    initRspPkt(&rspPkt, cmdPkt, RSP_OK);

    int i;

    for(i = 0; i < OfflineCommand_count; i++)
    {
        if(commData.pendingCommands[i].active)
        {
            addStringParamToRspPkt(&rspPkt, OfflineCommandNames[i]);
        }
    }

    if(rspPkt.paramCount == MIN_RSP_PARAM_COUNT)
    {
        addStringParamToRspPkt(&rspPkt, "Idle");
    }

    sendRspPkt(&rspPkt);
}

static void chGetFirmwareVersion(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    addParamToRspPkt(&rspPkt, "%s", FW_VER_APP);
    addParamToRspPkt(&rspPkt, "%s", __DATE__);
    addParamToRspPkt(&rspPkt, "%s", __TIME__);
    sendRspPkt(&rspPkt);
}

static void chGetFPGAVersion(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    addParamToRspPkt(&rspPkt, "%lu.%lu", FPGA_MAJOR_VER(), FPGA_MINOR_VER());
    sendRspPkt(&rspPkt);
}

static void chGetStatus(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;
    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    int i;

    for(i = 0; i < OfflineCommand_count; i++)
    {
        if(commData.pendingCommands[i].active)
        {
            addStringParamToRspPkt(&rspPkt, OfflineCommandNames[i]);
        }
    }

    if(rspPkt.paramCount == MIN_RSP_PARAM_COUNT)
    {
        addStringParamToRspPkt(&rspPkt, "Idle");
    }

    sendRspPkt(&rspPkt);
}

static void chGoToIdle(CmdPkt* cmdPkt)
{
    sendRspOk(cmdPkt);
}

static void chInitSystem(CmdPkt* cmdPkt)
{
    if(!checkPendingCmds(cmdPkt, OfflineCommand_InitializeSystem, 0)) return;

    registerAndSendRspPending(OfflineCommand_InitializeSystem, cmdPkt);
    initSystem(signalOfflineTaskCompleteCallback, OfflineCommand_InitializeSystem);
}

static void chMoveRelative(CmdPkt* cmdPkt)
{
    StepperMotor motor;
    int steps;

    // Check Motor Parameter
    if(!validateMotorParameter(cmdPkt, 0, &motor))
    {
        return;
    }

    // Check steps parameter
    if(!validateIntValue(cmdPkt, cmdPkt->params[1], &steps))
    {
        return;
    }

    // Move Motor
    stepper_move_relative( motor, steps );
    sendRspOk(cmdPkt);
}

static void chReboot(CmdPkt* cmdPkt)
{
    sendRspOk(cmdPkt);
    mdelay(100);

    reboot();
}

static void chReset(CmdPkt* cmdPkt)
{
    sendRspOk(cmdPkt);
}

static void chSendErrorMessage(CmdPkt* cmdPkt)
{
    if(cmdPkt->paramCount < 1)
    {
        sendRspStatusInvalidParameterCount(cmdPkt, 1, MAX_PARAMS_COUNT);
        return;
    }

    int errorCode;
    if(!validateIntParameterRange(cmdPkt, 0, 0, num_errors - 1, &errorCode)) return;

    sendRspOk(cmdPkt);

    RspPkt errorPkt;
    initIpErrorPkt(&errorPkt, errorCode);

    if(cmdPkt->paramCount > 1)
    {
        addStringParamToRspPkt(&errorPkt, "-");

        unsigned i;
        for(i = 1; i < cmdPkt->paramCount; ++i)
        {
            addStringParamToRspPkt(&errorPkt, cmdPkt->params[i]);
        }
    }

    sendRspPkt(&errorPkt);
}

static void chSendLogMessage(CmdPkt* cmdPkt)
{
    sendRspOk(cmdPkt);

    RspPkt logPkt;
    initIpLogPkt(&logPkt);

    unsigned i;
    for(i = 0; i < cmdPkt->paramCount; ++i)
    {
        addStringParamToRspPkt(&logPkt, cmdPkt->params[i]);
    }

    sendRspPkt(&logPkt);
}

static void chTestGetSensors(CmdPkt* cmdPkt)
{
    printf("----------Sensor readings----------\n");
    stepper_set_address_hw_UFR(0);
    printf("00 - Home Sensor - %d Alt Sensor  - %d\n",
           stepper_get_home_sensor_hw_UFR(), stepper_get_alt_sensor_hw_UFR());

    stepper_set_address_hw_UFR(1);
    printf("01 - Home Sensor - %d Alt Sensor  - %d\n",
           stepper_get_home_sensor_hw_UFR(), stepper_get_alt_sensor_hw_UFR());

    stepper_set_address_hw_UFR(2);
    printf("10 - Home Sensor - %d Alt Sensor  - %d\n",
           stepper_get_home_sensor_hw_UFR(), stepper_get_alt_sensor_hw_UFR());

    stepper_set_address_hw_UFR(3);
    printf("11 - Home Sensor - %d Alt Sensor  - %d\n\n",
           stepper_get_home_sensor_hw_UFR(), stepper_get_alt_sensor_hw_UFR());
    sendRspOk(cmdPkt);
}

static void chTestGetSwitch(CmdPkt* cmdPkt)
{
    printf("----------Switch signal readings----------\n"
           "L In - %d L Out - %d\n"
           "F In - %d F Out - %d\n"
           "R In - %d R Out - %d\n"
           "B In - %d B Out - %d\n"
           "U In - %d U Out - %d\n"
           "D In - %d D Out - %d\n\n", 
            IsSwitchTriggered(LInSwitch), IsSwitchTriggered(LOutSwitch),
            IsSwitchTriggered(FInSwitch), IsSwitchTriggered(FOutSwitch),
            IsSwitchTriggered(RInSwitch), IsSwitchTriggered(ROutSwitch),
            IsSwitchTriggered(BInSwitch), IsSwitchTriggered(BOutSwitch),
            IsSwitchTriggered(UInSwitch), IsSwitchTriggered(UOutSwitch),
            IsSwitchTriggered(DInSwitch), IsSwitchTriggered(DOutSwitch));
    sendRspOk(cmdPkt);
}

static void chUpgradeFirmware(CmdPkt* cmdPkt)
{
    uint32 fileSize;
    uint32 fileCRC;

    if(!validateUnsignedValue(cmdPkt, cmdPkt->params[0], &fileSize))
    {
        return;
    }

    if(!validateUnsignedValue(cmdPkt, cmdPkt->params[1], &fileCRC))
    {
        return;
    }

    // Verify image and upgrade
    if(verifyFirmwareImage(fileSize, fileCRC))
    {
        sendRspOk(cmdPkt);
        mdelay(1000);
        updateFirmware(fileSize, fileCRC);
    }
    else
    {
        sendRspStatusInvalidImage(cmdPkt, NULL);
    }
}

static void chUpgradeFPGA(CmdPkt* cmdPkt)
{
    uint32 fileSize;
    uint32 fileCRC;

    if(!validateUnsignedValue(cmdPkt, cmdPkt->params[0], &fileSize))
    {
        return;
    }

    if(!validateUnsignedValue(cmdPkt, cmdPkt->params[1], &fileCRC))
    {
        return;
    }

    // Verify image and upgrade
    if(verifyFPGAImage(fileSize, fileCRC))
    {
        sendRspOk(cmdPkt);
        mdelay(1000);
        updateFPGA(fileSize, fileCRC);
    }
    else
    {
        sendRspStatusInvalidImage(cmdPkt, NULL);
    }
}

// EOF
