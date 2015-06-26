/////////////////////////////////////////////////////////////
//
//  serial.c
//
//  Serial driver for the command protocol
//

#include <serial.h>
#include <xparameters.h>
#include <xuartlite.h>
#include <assert.h>
#include <interrupt.h>
#include <lcd.h>
#include <string.h>
#include <timer.h>
#include <os.h>
#include <stdio.h>
#include <processor.h>
#include <signal.h>

///////////////////////////////////////////////////
// Constants

#define RX_BUFFER_SIZE  MAX_RX_DATA_SIZE
#define TX_BUFFER_SIZE  (MAX_TX_DATA_SIZE + 2)  //Add 2 for packet frame characters

#define TX_BUFFER_COUNT 20

#define FRAME_START_CHAR '['
#define FRAME_END_CHAR   ']'

#define UART_ERROR_CHECK_TIMEOUT_ms    10


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


typedef struct
{
    volatile unsigned rxFifoReg;
    volatile unsigned txFifoReg;
    volatile unsigned statusReg;
    volatile unsigned controlReg;
} UartLiteRegs;


typedef struct TxBuffer TxBuffer;
struct TxBuffer
{
    uint8     data[TX_BUFFER_SIZE];
    uint8*    dataPtr;
    unsigned  count;
    TxBuffer* next;
};


typedef struct
{
    TxBuffer* head;
    TxBuffer* tail;
    unsigned  count;
} TxQueue;


typedef struct
{
    bool     running;
    Timer    timer;
    unsigned startTxCount;
} ErrorCheck;


typedef struct
{
    UartLiteRegs* regs;

    SerialPort    port;
    
    uint8         interruptId;

    RxState       rxState;
    unsigned      rxCount;
    uint8         rxBuffer[RX_BUFFER_SIZE];
    Signal*       rxSignal;

    TxBuffer      txBuffers[TX_BUFFER_COUNT];
    TxBuffer*     txBuffer;
    TxQueue       freeQueue;
    TxQueue       txQueue;
    
    unsigned      txCount;
    
    ErrorCheck    errorCheck;
} Uart;


typedef struct
{
    Uart sbcUart;
    Uart debugUart;
} SerialData;



///////////////////////////////////////////////////
// Local function prototypes

static bool      uartErrorCheckTask(void* reference);

static void      initUart(Uart* uart, SerialPort port, uint32 regsBaseAddr, uint8 interruptId);
static void      resetUart(Uart* uart);

static Uart*     getUart(SerialPort port);

static void      disableUartInterrupts(Uart* uart);
static void      enableUartInterrupts(Uart* uart);

static void      initTxQueue(TxQueue* queue);
static TxBuffer* dequeueTxBuffer(TxQueue* queue);
static void      enqueueTxBuffer(TxQueue* queue, TxBuffer* buffer);

static void      updateTxFifo(Uart* uart);

static void      uartInterruptHandler(void* callBackRef);
static void      rxHandler(Uart* uart);
static void      txHandler(Uart* uart);



///////////////////////////////////////////////////
// Local data

static SerialData serialData;



///////////////////////////////////////////////////
// Interface functions

void serialInit()
{
    initUart(&serialData.sbcUart, SBC_SERIAL_PORT, XPAR_SBC_UART_BASEADDR, XPAR_INTC_SBC_UART_INTERRUPT_INTR);
    registerInterruptHandler(XPAR_INTC_SBC_UART_INTERRUPT_INTR, (XInterruptHandler)uartInterruptHandler, &serialData.sbcUart);
    enableInterrupt(XPAR_INTC_SBC_UART_INTERRUPT_INTR);
}



void setSerialPortRxSignal(SerialPort port, Signal* signal)
{
    ASSERT(signal);
    
    Uart* uart = getUart(port);
    
    disableUartInterrupts(uart);
    {
       uart->rxSignal = signal;
    }
    enableUartInterrupts(uart);
}



unsigned serialGetPkt(SerialPort port, uint8* pkt, unsigned maxLength)
{
    ASSERT(pkt);

    unsigned count = 0;
    Uart* uart = getUart(port);
    disableUartInterrupts(uart);
    {
        if(uart->rxState == RxComplete)
        {
            if(uart->rxCount <= maxLength)
            {
                count = uart->rxCount;
                memcpy(pkt, uart->rxBuffer, count);
            }
            //Else - Error: unable to copy whole buffer

            uart->rxState = RxIdle;
        }
    }
    enableUartInterrupts(uart);

    return count;
}



bool serialSendPkt(SerialPort port, const uint8* pkt, unsigned length, uint32 timeout_us)
{
    ASSERT(length <= MAX_TX_DATA_SIZE);
    ASSERT(length  > 0);

    Uart* uart = getUart(port);
    Timer timer;
    startTimer(&timer, USEC_TO_TICKS(timeout_us));
    
    do
    {
        TxBuffer* buffer = dequeueTxBuffer(&uart->freeQueue);
        if(buffer)
        {
            // Load buffer including packet frame
            buffer->data[0] = FRAME_START_CHAR;
            memcpy(&buffer->data[1], pkt, length);
            buffer->data[1 + length] = FRAME_END_CHAR;
            buffer->count = length + 2; // Add 2 for frame characters

            enqueueTxBuffer(&uart->txQueue, buffer);
            updateTxFifo(uart);
            
            return true;
        }
    } while(!timerExpired(&timer));
    
    

    // Couldn't enqueue TX packet - Start up UART Error Check Task
    CRData crdata = enterCriticalRegion();
    {
        if(!uart->errorCheck.running)
        {
            uart->errorCheck.running = true;
            uart->errorCheck.startTxCount = uart->txCount;
            
            startTimer(&uart->errorCheck.timer, MSEC_TO_TICKS(UART_ERROR_CHECK_TIMEOUT_ms));
            scheduleTask(uartErrorCheckTask, uart, false);

            printf("Starting UART Error Check Task\n");
        }
    }
    exitCriticalRegion(crdata);


    return false;
}




///////////////////////////////////////////////////
// Local functions

static bool uartErrorCheckTask(void* reference)
{
    ASSERT(reference);
    
    Uart* uart     = (Uart*)reference;
    bool  runAgain = true;
    
    printf("Checking UART error...\n");

    CRData crdata = enterCriticalRegion();
    {
        if(uart->errorCheck.startTxCount != uart->txCount)
        {
            printf("UART OK\n");
            runAgain                 = false;
            uart->errorCheck.running = false;
        }
        else if(timerExpired(&uart->errorCheck.timer))
        {
            printf("Resetting UART\n");
            resetUart(uart);
            runAgain                 = false;
            uart->errorCheck.running = false;
        }
    }
    exitCriticalRegion(crdata);
    
    return runAgain;
}



static void initUart(Uart* uart, SerialPort port, uint32 regsBaseAddr, uint8 interruptId)
{
    uart->regs               = (UartLiteRegs*)regsBaseAddr;
    uart->port               = port;
    uart->rxSignal           = NULL;
    uart->interruptId        = interruptId;
    uart->rxCount            = 0;
    uart->rxState            = RxIdle;
    uart->txBuffer           = NULL;
    uart->txCount            = 0;
    uart->errorCheck.running = false;
    
    initTxQueue(&uart->freeQueue);
    initTxQueue(&uart->txQueue);
    
    unsigned i;
    for(i = 0; i < TX_BUFFER_COUNT; ++i)
    {
        enqueueTxBuffer(&uart->freeQueue, &uart->txBuffers[i]);
    }
    
    resetUart(uart);
}



static void resetUart(Uart* uart)
{
    ASSERT(uart);
    
    CRData crdata = enterCriticalRegion();
    {
        uart->rxCount = 0;
        uart->rxState = RxIdle;
        
        if(uart->txBuffer)
        {
            enqueueTxBuffer(&uart->freeQueue, uart->txBuffer);
            uart->txBuffer  = NULL;
        }

        uart->regs->controlReg = CTRL_RST_TX_FIFO_MASK |
                                 CTRL_RST_RX_FIFO_MASK |
                                 CTRL_ENABLE_INTR_MASK;
    }
    exitCriticalRegion(crdata);
    
    updateTxFifo(uart);
}



static Uart* getUart(SerialPort port)
{
    switch(port)
    {
        case SBC_SERIAL_PORT:
            return &serialData.sbcUart;

        default:
            //Error: invalid port specified
            ASSERT(false);
    }

    return NULL;
}



static void disableUartInterrupts(Uart* uart)
{
    disableInterrupt(uart->interruptId);
}



static void enableUartInterrupts(Uart* uart)
{
    enableInterrupt(uart->interruptId);
}




static void initTxQueue(TxQueue* queue)
{
    ASSERT(queue);
    
    queue->head  = NULL;
    queue->tail  = NULL;
    queue->count = 0;
}



static TxBuffer* dequeueTxBuffer(TxQueue* queue)
{
    ASSERT(queue);
    
    TxBuffer* buffer = NULL;
    
    CRData crdata = enterCriticalRegion();
    {
        if(queue->head)
        {
            buffer = queue->head;
            queue->head = queue->head->next;
            
            if(!queue->head)
            {
                queue->tail = NULL;
            }
            
            buffer->next = NULL;
            --queue->count;
        }
    }
    exitCriticalRegion(crdata);
    
    return buffer;
}



static void enqueueTxBuffer(TxQueue* queue, TxBuffer* buffer)
{
    ASSERT(queue);
    ASSERT(buffer);
    
    CRData crdata = enterCriticalRegion();
    {
        buffer->next = NULL;
        
        if(!queue->tail)
        {
            queue->head = buffer;
        }
        else
        {
            queue->tail->next = buffer;
        }
        
        queue->tail = buffer;
        ++queue->count;
    }
    exitCriticalRegion(crdata);
}



static void updateTxFifo(Uart* uart)
{
    CRData crdata = enterCriticalRegion();
    {
        while(!(uart->regs->statusReg & STATUS_TX_FIFO_FULL_MASK))
        {
            if(!uart->txBuffer)
            {
                uart->txBuffer = dequeueTxBuffer(&uart->txQueue);
                
                if(!uart->txBuffer)
                {
                    break;
                }
                
                uart->txBuffer->dataPtr = uart->txBuffer->data;
            }

            if(!uart->txBuffer->count)
            {
                enqueueTxBuffer(&uart->freeQueue, uart->txBuffer);
                uart->txBuffer = NULL;
                continue;
            }

            uart->regs->txFifoReg = *uart->txBuffer->dataPtr;
            ++uart->txBuffer->dataPtr;
            --uart->txBuffer->count;
        }
    }
    exitCriticalRegion(crdata);
}



static void uartInterruptHandler(void* callBackRef)
{
    ASSERT(callBackRef);

    Uart* uart = (Uart*)callBackRef;
    uint32 status = uart->regs->statusReg;

    if(status & STATUS_TX_FIFO_EMPTY_MASK)
    {
        txHandler(uart);
    }

    if(status & STATUS_RX_FIFO_VALID_DATA_MASK)
    {
        rxHandler(uart);
    }

    // TODO: Handle other interrupts
    //    STATUS_RX_FIFO_FULL_MASK
    //    STATUS_TX_FIFO_FULL_MASK
    //    STATUS_INTR_ENABLED_MASK
    //    STATUS_OVERRUN_ERROR_MASK
    //    STATUS_FRAME_ERROR_MASK
    //    STATUS_PARITY_ERROR_MASK
}



static void rxHandler(Uart* uart)
{
//    puts("RX Handler");
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
                
                if(uart->rxSignal)
                {
                    setSignal(uart->rxSignal);
                }
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
    updateTxFifo(uart);
}


// EOF
