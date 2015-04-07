/////////////////////////////////////////////////////////////
//
//  spi.c
//
//  SPI Driver
//

#include <spi.h>
#include <xparameters.h>
#include <processor.h>
#include <assert.h>
#include <interrupt.h>
#include <string.h>
#include <timer.h>



///////////////////////////////////////////////////
// Constants


///////////////////////////////////////////////////
// Local types and macros



//////////////////////////////////////////////////
// Local function prototypes

static void dataCopy(SpiInterface* interface, uint8* toBuf, uint8* fromBuf, uint32 count);

static void spiInterruptHandler(void *callBackRef, Xuint32 statusEvent, Xuint32 byteCount);



///////////////////////////////////////////////////
// Local data



///////////////////////////////////////////////////
// Interface functions

void spiInterfaceInit(SpiInterface* interface, uint16 deviceId, uint8 interruptId, uint32 xilOptions, uint32 options)
{
    ASSERT(interface);
    
    interface->state            = SPI_STATE_IDLE;
    interface->invertedDataCopy = !!(options & SPI_INVERTED_DATA);

    XStatus status;

    status = XSpi_Initialize(&interface->hwInterface, deviceId);
    ASSERT(status == XST_SUCCESS);

    status = XSpi_SelfTest(&interface->hwInterface);
    ASSERT(status == XST_SUCCESS);

    status = XSpi_SetOptions(&interface->hwInterface, xilOptions);

    ASSERT(status == XST_SUCCESS);

    status = XSpi_SetSlaveSelect(&interface->hwInterface, 0);
    ASSERT(status == XST_SUCCESS);

 
    XSpi_SetStatusHandler(&interface->hwInterface, interface, (XSpi_StatusHandler)spiInterruptHandler);

    registerInterruptHandler(interruptId, (XInterruptHandler)XSpi_InterruptHandler, interface);
    enableInterrupt(interruptId);

    XSpi_Start(&interface->hwInterface);

    // \todo: Figure out why we have to do this!
    // Enable the first slave.
    XSpi_SetSlaveSelect(&interface->hwInterface, 0x01);
}



bool spiWaitForIdle(SpiInterface* interface, uint32 timeout_us)
{
    ASSERT(interface);

    Timer timer;
    
    startTimer(&timer, USEC_TO_TICKS(timeout_us));
    
    while(1)
    {
        SpiState state;
        
        CRData crdata = enterCriticalRegion();
        {
            state = interface->state;
        }
        exitCriticalRegion(crdata);

        if(state == SPI_STATE_IDLE || state == SPI_STATE_DONE || state == SPI_STATE_DONE_WITH_ERRORS)
        {
            return true;
        }
        else if(timerExpired(&timer))
        {
            return false;
        }
    }
    
    return false;
}



bool spiWaitForReady(SpiInterface* interface, uint32 timeout_us)
{
    ASSERT(interface);

    Timer timer;
    
    startTimer(&timer, USEC_TO_TICKS(timeout_us));
    
    while(1)
    {
        SpiState state;
        
        CRData crdata = enterCriticalRegion();
        {
            state = interface->state;
        }
        exitCriticalRegion(crdata);

        if(state == SPI_STATE_DONE || state == SPI_STATE_DONE_WITH_ERRORS)
        {
            return true;
        }
        else if(timerExpired(&timer))
        {
            return false;
        }
    }
    
    return false;
}



SpiStatus spiStartTransmit(SpiInterface* interface, uint8* writeBuffer, uint16 writeLength)
{
    SpiStatus status;
    
    ASSERT(interface);
    ASSERT(writeBuffer);
    ASSERT(writeLength <= SPI_BUFFER_SIZE);

    CRData crdata = enterCriticalRegion();
    {
        if(interface->state == SPI_STATE_TRANSMITTING)
        {
            status = SPI_STATUS_ERROR_TX_IN_PROCESS;
        }
        else
        {
            dataCopy(interface, interface->writeBuffer, writeBuffer, writeLength);
            interface->writeLength = writeLength;
            interface->state       = SPI_STATE_TRANSMITTING;
            XSpi_Transfer(&interface->hwInterface, interface->writeBuffer, interface->readBuffer, writeLength);
            status = SPI_STATUS_TX_STARTED;
        }
    }
    exitCriticalRegion(crdata);

    return status;
}


SpiStatus spiReadData(SpiInterface* interface, uint8* readBuffer, uint16 maxReadLength, uint16* readLength)
{
    SpiStatus status;
    
    ASSERT(interface);
    ASSERT(readBuffer);
    ASSERT(maxReadLength <= SPI_BUFFER_SIZE);
    ASSERT(readLength);
    
    CRData crdata = enterCriticalRegion();
    {
        if(interface->state == SPI_STATE_TRANSMITTING)
        {
            status = SPI_STATUS_ERROR_TX_IN_PROCESS;
        }
        else if(interface->state == SPI_STATE_DONE_WITH_ERRORS)
        {
            status = SPI_STATUS_TX_ERROR;
        }
        else if(interface->state == SPI_STATE_DONE)
        {
            if(maxReadLength < interface->readLength)
            {
                status      = SPI_STATUS_ERROR_RX_OVERRUN;
                *readLength = maxReadLength;
            }
            else
            {
                status      = SPI_STATUS_RX_OK;
                *readLength = interface->readLength;
            }

            dataCopy(interface, readBuffer, interface->readBuffer, *readLength);

            interface->state = SPI_STATE_IDLE;
        }
        else
        {
            status      = SPI_STATUS_ERROR_NO_RX_AVAILABLE;
            *readLength = 0;
        }
    }
    exitCriticalRegion(crdata);
    
    return status;
}



SpiStatus spiWaitAndStartTransmit(SpiInterface* interface, uint8* writeBuffer, uint16 writeLength, uint32 timeout_us)
{
    ASSERT(interface);
    spiWaitForIdle(interface, timeout_us);
    return spiStartTransmit(interface, writeBuffer, writeLength);
}



SpiStatus spiWaitAndReadData(SpiInterface* interface, uint8* readBuffer, uint16 maxReadLength, uint16* readLength, uint32 timeout_us)
{
    ASSERT(interface);
    spiWaitForReady(interface, timeout_us);
    return spiReadData(interface, readBuffer, maxReadLength, readLength);
}



void spiReset(SpiInterface* interface)
{
    ASSERT(interface);
    XSpi_Reset(&interface->hwInterface);
}


void spiSetOptions(SpiInterface* interface, uint32 xilOptions)
{
    ASSERT(XSpi_SetOptions(&interface->hwInterface, xilOptions) == XST_SUCCESS);
}


///////////////////////////////////////////////////
// Local functions

static void dataCopy(SpiInterface* interface, uint8* toBuf, uint8* fromBuf, uint32 count)
{
    if(interface->invertedDataCopy)
    {
        int i;
        for(i = 0; i < count; i++)
        {
            toBuf[i] = ~fromBuf[i];
        }
    }
    else
    {
        memcpy(toBuf, fromBuf, count);
    }
}



static void spiInterruptHandler(void *callBackRef, Xuint32 statusEvent, Xuint32 byteCount)
{
    ASSERT(callBackRef);
    
    SpiInterface* interface = (SpiInterface*)callBackRef;
    
    if(statusEvent != XST_SPI_TRANSFER_DONE)
    {
        interface->state = SPI_STATE_DONE_WITH_ERRORS;
        interface->readLength = 0;
    }
    else
    {
        interface->state = SPI_STATE_DONE;
        interface->readLength = byteCount;
    }
}


// EOF
