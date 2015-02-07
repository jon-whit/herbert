/////////////////////////////////////////////////////////////
//
//  comm.c
//
//  Communication Protocol Processor
//
//  Copyright 2008, 2009 Idaho Technology
//  Created by Brett Gilbert
//
//
//  Commands:
//    CMD <Alias> [params]                               - Description
//    --------------------------------------------------------------------------------------------------------
//    GetFirmwareVersion <RV>                            - Get firmware version
//    FileSend [File Offset] [Data]                      - Upload firmware file portion to master or slave
//    Upgrade  [File Size] [File CRC]                    - Upgrade firmware with uploaded image on master or slave
//



#include <comm.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <processor.h>
#include <timer.h>
#include <serial.h>
#include <update.h>
#include <version.h>
#include <errors.h>
#include <assert.h>
#include <system.h>



///////////////////////////////////////////////////
// Options

#define USE_SEQUENCE_NUMBER 1
#define DISPLAY_CMD_PKTS    0



///////////////////////////////////////////////////
// Constants

#define COMM_PROCESS_HOST_PERIOD_ms        10
#define COMM_PROCESS_SLAVE_PERIOD_ms       10

#define COMM_BYTE_TIME_us                  87
#define DEFAULT_COMM_SLAVE_RSP_TIMEOUT_ms  25
#define UPGRADE_COMM_SLAVE_RSP_TIMEOUT_ms  200

#define COMM_CMD_BUF_SIZE    1024
#define COMM_RSP_BUF_SIZE    1024
#define MAX_PARAMS_COUNT     30

#define MAX_TX_ATTEMPTS      5
#define TX_BUSY_DELAY_us     100

#define TO_MASTER_FROM_HOST_KEY       "MH"
#define TO_HOST_FROM_MASTER_KEY       "HM"

#define RSP_OK                        "OK"
#define RSP_ERROR                     "ER"
#define RSP_PENDING                   "PE"
#define RSP_INFORMATION_PKT           "IP"



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
} RspPkt;



typedef struct
{
    char* cmd;
    int   paramCount;
    void  (*cmdHandler)(CmdPkt* cmdPkt);
} CommCommand;



typedef struct
{
} CommData;



///////////////////////////////////////////////////
// Local function prototypes

static bool parseCmdPkt(CmdPkt* cmdPkt);
static void processCmdPkt(CmdPkt* cmdPkt);

static void initRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, const char* status);
static void addParamToRspPkt(RspPkt* rspPkt, const char *format, ...);
static void sendRspPkt(RspPkt* rspPkt);



// Status functions
static void sendRspOk(CmdPkt* cmdPkt);
static void sendRspError(CmdPkt* cmdPkt, unsigned errorCode, const char* errorString);

static void sendRspStatusOutOfRange(CmdPkt* cmdPkt);
static void sendRspStatusInvalidParameter(CmdPkt* cmdPkt);
static void sendRspStatusInvalidParameterCount(CmdPkt* cmdPkt);
static void sendRspStatusUnknownCommand(CmdPkt* cmdPkt);
static void sendRspStatusInvalidImage(CmdPkt* cmdPkt);
static void sendRspStatusMissingFileData(CmdPkt* cmdPkt);
static void sendRspStatusInvalidFileData(CmdPkt* cmdPkt);


// String parsing functions
static bool validateIntValue(CmdPkt* cmdPkt, const char* str, int* value);
static bool validateUnsignedValue(CmdPkt* cmdPkt, const char* str, uint32* value);


// Command Handlers
static void chFileSend(CmdPkt* cmdPkt);
static void chUpgrade(CmdPkt* cmdPkt);

static void chFirmwareVersion(CmdPkt* cmdPkt);
static void chFPGAVersion(CmdPkt* cmdPkt);



///////////////////////////////////////////////////
// Local data

static const CommCommand commCommands[] =
{   // Cmd,                 Param Count, Cmd Handler,
    { "GetFirmwareVersion", 0,           chFirmwareVersion,   },
    { "RV",                 0,           chFirmwareVersion,   },
    { "GetFPGAVersion",     0,           chFPGAVersion,       },

    { "FileSend",           2,           chFileSend,          },
    { "Upgrade",            2,           chUpgrade,           },

    // End of table marker
    { NULL, 0, NULL}
};



static CommData commData;



///////////////////////////////////////////////////
// Interface functions

void commInit()
{
    memset(&commData, 0, sizeof(commData));
}



void commProcess()
{
    CmdPkt cmdPkt;

    cmdPkt.bufByteCount = serialGetPkt((uint8*)cmdPkt.buf, COMM_CMD_BUF_SIZE - 1);

    if(cmdPkt.bufByteCount)
    {
        cmdPkt.buf[cmdPkt.bufByteCount] = 0;
        
//        printf("RX: %s\n", cmdPkt.buf);

        if(parseCmdPkt(&cmdPkt))
        {
            processCmdPkt(&cmdPkt);
        }
    }
}



///////////////////////////////////////////////////
// Local functions

static bool parseCmdPkt(CmdPkt* cmdPkt)
{
    ASSERT(cmdPkt);

    cmdPkt->dstSrc     = NULL;
    cmdPkt->seqNum     = NULL;
    cmdPkt->cmd        = NULL;
    cmdPkt->crc        = NULL;
    cmdPkt->paramCount = 0;

    if(cmdPkt->buf[0] == ' ' || !isprint(cmdPkt->buf[0]))
    {
        // First character is bad
        return false;
    }

    //TODO: get CRC here

    unsigned i;
    bool     startOfParam = true;
    char*    bufPtr;

    for(i = 0, bufPtr = cmdPkt->buf;
        i < cmdPkt->bufByteCount && i < sizeof(cmdPkt->buf) - 1;
        i++, bufPtr++)
    {
        if(!isprint(*bufPtr))
        {
            // Invalid character
            return false;
        }

        if(*bufPtr == ' ')
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

                #if USE_SEQUENCE_NUMBER
                    else if(!cmdPkt->seqNum)
                    {
                        cmdPkt->seqNum = bufPtr;
                    }
                #endif

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
       #if USE_SEQUENCE_NUMBER
           !cmdPkt->seqNum ||
       #endif
       !cmdPkt->cmd    ||
       cmdPkt->paramCount == 0)
    {
        return false;
    }

    cmdPkt->crc = cmdPkt->params[--cmdPkt->paramCount];

    //TODO: verify CRC here

    #if DISPLAY_CMD_PKTS
        printf("Cmd Pkt:\n");
        printf("  Dst/Src   = %s\n", cmdPkt->dstSrc);
        #if USE_SEQUENCE_NUMBER
            printf("  Seq Num   = %s\n", cmdPkt->seqNum);
        #endif
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
            if(cmdPkt->paramCount == commCommandIterator->paramCount)
            {
                commCommandIterator->cmdHandler(cmdPkt);
            }
            else
            {
                sendRspStatusInvalidParameterCount(cmdPkt);
            }

            return;
        }

        commCommandIterator++;
    }

    sendRspStatusUnknownCommand(cmdPkt);
}



static void addParamToRspPkt(RspPkt* rspPkt, const char *format, ...)
{
    if(rspPkt->paramCount > 0 && rspPkt->bufByteCount < COMM_RSP_BUF_SIZE - 2)
    {
        // Add space
        *rspPkt->bufPtr++ = ' ';
        rspPkt->bufByteCount++;
    }

    int     count;
    va_list argp;

    va_start (argp, format);

    //TODO: use vsnprint
    count = vsprintf(rspPkt->bufPtr, format, argp) - 1; //Subtract null termination
    va_end (argp);

    rspPkt->bufPtr    += count;
    rspPkt->bufByteCount += count;

    ASSERT(rspPkt->bufByteCount <= COMM_RSP_BUF_SIZE);

    rspPkt->paramCount++;
}



static void sendRspPkt(RspPkt* rspPkt)
{
    //TODO: set real CRC
    addParamToRspPkt(rspPkt, "0");
    
//    printf("TX: '%s'\n", rspPkt->buf);

    while(!serialSendPkt((uint8*)rspPkt->buf, rspPkt->bufByteCount))
    {
        udelay(TX_BUSY_DELAY_us);
    }
    
    while(txProcessing())
    {
        serialProcess();
    }
}



static void initRspPkt(RspPkt* rspPkt, CmdPkt* cmdPkt, const char* status)
{
    ASSERT(rspPkt);
    ASSERT(cmdPkt);
    ASSERT(status);

    rspPkt->bufByteCount = 0;
    rspPkt->paramCount   = 0;
    rspPkt->bufPtr       = rspPkt->buf;

    addParamToRspPkt(rspPkt, TO_HOST_FROM_MASTER_KEY);
    #if USE_SEQUENCE_NUMBER
        addParamToRspPkt(rspPkt, cmdPkt->seqNum);
    #endif
    addParamToRspPkt(rspPkt, cmdPkt->cmd);
    addParamToRspPkt(rspPkt, status);
}



static void sendRspOk(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    sendRspPkt(&rspPkt);
}



static void sendRspError(CmdPkt* cmdPkt,  ErrorCodes errorCode, const char* errorString)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_ERROR);
    addParamToRspPkt(&rspPkt, "%03d", errorCode);

    if(errorString)
    {
        addParamToRspPkt(&rspPkt, errorString);
    }

    sendRspPkt(&rspPkt);
}



static void sendRspStatusOutOfRange(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_parameterOutOfRange, error_strings[err_parameterOutOfRange]);
}



static void sendRspStatusInvalidParameter(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidParameter, error_strings[err_invalidParameter]);
}



static void sendRspStatusInvalidParameterCount(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidParameterCount, error_strings[err_invalidParameterCount]);
}



static void sendRspStatusUnknownCommand(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_unknownCommand, error_strings[err_unknownCommand]);
}



static void sendRspStatusInvalidImage(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidUpgradeImage, error_strings[err_invalidUpgradeImage]);
}



static void sendRspStatusMissingFileData(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_missingFileData, error_strings[err_missingFileData]);
}



static void sendRspStatusInvalidFileData(CmdPkt* cmdPkt)
{
    sendRspError(cmdPkt, err_invalidFileData, error_strings[err_invalidFileData]);
}



static bool validateIntValue(CmdPkt* cmdPkt, const char* str, int* value)
{
    bool negative = false;

    *value = 0;

    if(*str == '-')
    {
        negative = true;
        str++;
    }

    while(*str)
    {
        if(!isdigit(*str))
        {
            sendRspStatusInvalidParameter(cmdPkt);
            return false;
        }

        *value *= 10;
        *value += *str++ - '0';
    }

    if(negative)
    {
        *value *= -1;
    }

    return true;
}



static bool validateUnsignedValue(CmdPkt* cmdPkt, const char* str, uint32* value)
{
    *value = 0;

    while(*str)
    {
        if(!isdigit(*str))
        {
            sendRspStatusInvalidParameter(cmdPkt);
            return false;
        }

        *value *= 10;
        *value += *str++ - '0';
    }

    return true;
}



static bool validateIntParameterRange(CmdPkt* cmdPkt, int paramIndex, int minValue, int maxValue, int* value)
{
    ASSERT(cmdPkt);
    ASSERT(value);
    ASSERT(paramIndex < cmdPkt->paramCount && paramIndex >= 0);

    if(!(validateIntValue(cmdPkt, cmdPkt->params[paramIndex], value)))
    {
        return false;
    }

    if(*value < minValue || *value > maxValue)
    {
        sendRspStatusOutOfRange(cmdPkt);
        return false;
    }

    return true;
}



///////////////////////////////////////////////////
// Command handler functions

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



static void chUpgrade(CmdPkt* cmdPkt)
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


    if(verifyFirmwareImage(fileSize, fileCRC))
    {
        sendRspOk(cmdPkt);
        mdelay(1000);
        updateFirmware(fileSize, fileCRC);
    }
    else
    {
        sendRspStatusInvalidImage(cmdPkt);
    }
}



static void chFirmwareVersion(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    addParamToRspPkt(&rspPkt, "Bootloader-%s", FW_VER_BOOT);
    addParamToRspPkt(&rspPkt, "%s", __DATE__);
    addParamToRspPkt(&rspPkt, "%s", __TIME__);
    sendRspPkt(&rspPkt);
}



static void chFPGAVersion(CmdPkt* cmdPkt)
{
    RspPkt rspPkt;

    initRspPkt(&rspPkt, cmdPkt, RSP_OK);
    addParamToRspPkt(&rspPkt, "%lu.%lu", FPGA_MAJOR_VER(), FPGA_MINOR_VER());
    sendRspPkt(&rspPkt);
}


// EOF
