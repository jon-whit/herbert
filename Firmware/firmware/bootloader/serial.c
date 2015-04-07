/////////////////////////////////////////////////////////////
//
//  serial.c
//
//  Serial driver for the command protocol
//

#include <serial.h>
#include <xparameters.h>
#include <xuartlite.h>
#include <processor.h>
#include <assert.h>
#include <lcd.h>
#include <string.h>



///////////////////////////////////////////////////
// Constants

#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 1024

#define FRAME_START_CHAR '['
#define FRAME_END_CHAR   ']'


// Register masks
#define CTRL_RST_TX_FIFO_MASK          0x00000001
#define CTRL_RST_RX_FIFO_MASK          0x00000002
#define CTRL_ENABLE_INTR_MASK          0x00000010

#define STATUS_RX_FIFO_VALID_DATA_MASK 0x00000001
#define STATUS_RX_FIFO_FULL_MASK       0x00000002
#define STATUS_TX_FIFO_EMPTY_MASK      0x00000004
#define STATUS_TX_FIFO_FULL_MASK       0x00000008
#define STATUS_INTR_ENABLED_MASK       0x00000010
#define STATUS_OVERRUN_ERROR_MASK      0x00000020
#define STATUS_FRAME_ERROR_MASK        0x00000040
#define STATUS_PARITY_ERROR_MASK       0x00000080



///////////////////////////////////////////////////
// Local types and macros

typedef enum
{
    RxIdle,
    RxInProgress,
    RxComplete
} RxState;

typedef enum
{
    TxIdle,
    TxInProgress,
    TxFinishing
} TxState;



typedef struct
{
    volatile unsigned rxFifoReg;
    volatile unsigned txFifoReg;
    volatile unsigned statusReg;
    volatile unsigned controlReg;
} UartLiteRegs;



typedef struct
{
    UartLiteRegs* regs;

    RxState       rxState;
    unsigned      rxCount;
    uint8         rxBuffer[RX_BUFFER_SIZE];

    TxState       txState;
    unsigned      txCount;
    uint8         txBuffer[TX_BUFFER_SIZE];
    uint8*        txDataPtr;
} Uart;



typedef struct
{
    Uart sbcUart;
} SerialData;



///////////////////////////////////////////////////
// Local function prototypes

static void  initUart(Uart* uart, uint32 regsBaseAddr);

static void  updateTxFifo(Uart* uart);

static void  rxHandler(Uart* uart);
static void  txHandler(Uart* uart);



///////////////////////////////////////////////////
// Local data

static SerialData serialData;



///////////////////////////////////////////////////
// Interface functions

void serialInit()
{
    initUart(&serialData.sbcUart, XPAR_SBC_UART_BASEADDR);

//    ASSERT(&serialData.sbcUart.regs->rxFifoReg  == (unsigned*)(DCE_UART_REGS_BASEADDR + 0x0));
//    ASSERT(&serialData.sbcUart.regs->txFifoReg  == (unsigned*)(DCE_UART_REGS_BASEADDR + 0x4));
//    ASSERT(&serialData.sbcUart.regs->statusReg  == (unsigned*)(DCE_UART_REGS_BASEADDR + 0x8));
//    ASSERT(&serialData.sbcUart.regs->controlReg == (unsigned*)(DCE_UART_REGS_BASEADDR + 0xc));
//
//    ASSERT(&serialData.dteUart.regs->rxFifoReg  == (unsigned*)(DTE_UART_REGS_BASEADDR + 0x0));
//    ASSERT(&serialData.dteUart.regs->txFifoReg  == (unsigned*)(DTE_UART_REGS_BASEADDR + 0x4));
//    ASSERT(&serialData.dteUart.regs->statusReg  == (unsigned*)(DTE_UART_REGS_BASEADDR + 0x8));
//    ASSERT(&serialData.dteUart.regs->controlReg == (unsigned*)(DTE_UART_REGS_BASEADDR + 0xc));
}



void serialProcess()
{
    if(serialData.sbcUart.regs->statusReg & STATUS_TX_FIFO_EMPTY_MASK)
    {
        txHandler(&serialData.sbcUart);
    }

    if(serialData.sbcUart.regs->statusReg & STATUS_RX_FIFO_VALID_DATA_MASK)
    {
        rxHandler(&serialData.sbcUart);
    }
}



bool txProcessing()
{
    return serialData.sbcUart.txState != TxIdle;
}



unsigned serialGetPkt(uint8* pkt, unsigned maxLength)
{
    ASSERT(pkt);
    
    unsigned count = 0;

    if(serialData.sbcUart.rxState == RxComplete)
    {
        if(serialData.sbcUart.rxCount <= maxLength)
        {
            count = serialData.sbcUart.rxCount;
            memcpy(pkt, serialData.sbcUart.rxBuffer, count);
        }
        //Else - Error: unable to copy whole buffer

        serialData.sbcUart.rxState = RxIdle;
    }

    return count;
}



bool serialSendPkt(uint8* pkt, unsigned length)
{
    bool success = false;

    ASSERT(length <= TX_BUFFER_SIZE);

    if(serialData.sbcUart.txState == TxIdle)
    {
        serialData.sbcUart.regs->controlReg = CTRL_RST_TX_FIFO_MASK;

        memcpy(serialData.sbcUart.txBuffer, pkt, length);
        serialData.sbcUart.txCount   = length;
        serialData.sbcUart.txDataPtr = serialData.sbcUart.txBuffer;

        updateTxFifo(&serialData.sbcUart);

        success = true;
    }

    return success;
}



///////////////////////////////////////////////////
// Local functions

static void initUart(Uart* uart, uint32 regsBaseAddr)
{
    uart->regs    = (UartLiteRegs*)regsBaseAddr;

    uart->rxCount = 0;
    uart->rxState = RxIdle;

    uart->txCount = 0;
    uart->txState = TxIdle;

    uart->regs->controlReg = CTRL_RST_TX_FIFO_MASK |
                             CTRL_RST_RX_FIFO_MASK |
                             CTRL_ENABLE_INTR_MASK;
}



static void updateTxFifo(Uart* uart)
{
    while(!(uart->regs->statusReg & STATUS_TX_FIFO_FULL_MASK))
    {
        switch(uart->txState)
        {
            case TxIdle:
                uart->regs->txFifoReg = FRAME_START_CHAR;
                uart->txState = TxInProgress;
                break;
            case TxInProgress:
                if(uart->txCount)
                {
                    uart->regs->txFifoReg = *uart->txDataPtr;
                    uart->txDataPtr++;
                    uart->txCount--;
                }
                else
                {
                    uart->regs->txFifoReg = FRAME_END_CHAR;
                    uart->txState = TxFinishing;
                    return;
                }
                break;
            case TxFinishing:
                //Pkt Completely transmitted
                uart->txState = TxIdle;
                return;
            default:
                //Invalid state
                ASSERT(false);
                break;
        }
    }
}



static void rxHandler(Uart* uart)
{
    while(uart->regs->statusReg & STATUS_RX_FIFO_VALID_DATA_MASK)
    {
        uint8 byte = uart->regs->rxFifoReg;

        if(uart->rxState == RxIdle)
        {
            if(byte == FRAME_START_CHAR)
            {
                // Pkt start detected
                uart->rxCount = 0;
                uart->rxState = RxInProgress;
            }
            // Else - no start of pkt - ignore
        }
        else if(uart->rxState == RxInProgress)
        {
            if(byte == FRAME_START_CHAR)
            {
                // Restart detected - clear pkt and start over
                uart->rxCount = 0;
            }
            else if(byte == FRAME_END_CHAR)
            {
                // End of pkt detected
                uart->rxState = RxComplete;
                
                uart->rxBuffer[uart->rxCount] = 0; // Null terminate
            }
            else
            {
                // Pkt byte
                if(uart->rxCount < RX_BUFFER_SIZE)
                {
                    uart->rxBuffer[uart->rxCount++] = byte;
                }
                // else - buffer full - drop data
            }
        }
        // else - uart->rxByte == RxComplete -- ignore data
    }
}



static void txHandler(Uart* uart)
{
    if(uart->txState != TxIdle)
    {
        updateTxFifo(uart);
    }
}


// EOF
