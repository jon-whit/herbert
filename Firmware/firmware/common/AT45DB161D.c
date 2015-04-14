/////////////////////////////////////////////////////////////
//
//  AT45DB161D.c
//
//  Atmel AT45DB161D Driver
//

#include <AT45DB161D.h>
#include <spi.h>
#include <xparameters.h>
#include <processor.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>



///////////////////////////////////////////////////
// Constants

enum OpCodes
{
    OP_BLOCK_ERASE                           = 0x50,
    OP_MAIN_MEMORY_PAGE_READ                 = 0xD2,
    OP_CONTINUOUS_ARRAY_READ_LOW_SPEED       = 0x03,
    OP_CONTINUOUS_ARRAY_READ_HIGH_SPEED      = 0x0B,
    OP_BUFFER1_READ_LOW_SPEED                = 0xD1,
    OP_BUFFER2_READ_LOW_SPEED                = 0xD3,
    OP_BUFFER1_READ_LOW                      = 0xD4,
    OP_BUFFER2_READ_LOW                      = 0xD6,
    
    OP_BUFFER1_WRITE                         = 0x84,
    OP_BUFFER2_WRITE                         = 0x87,
    OP_BUFFER1_TO_PAGE_PROG_WITH_ERASE       = 0x83,
    OP_BUFFER2_TO_PAGE_PROG_WITH_ERASE       = 0x86,
    
    OP_STATUS_REGISTER_READ                  = 0xD7,
};



#define TX_TIMEOUT_us         10000
#define RX_TIMEOUT_us         10000

#define MAX_SINGLE_READ_SIZE          (SPI_BUFFER_SIZE - 5)
#define MAX_SINGLE_BUFFER_WRITE_SIZE  (SPI_BUFFER_SIZE - 4)



///////////////////////////////////////////////////
// Local types and macros



///////////////////////////////////////////////////
// Local function prototypes

static bool sendAndReceiveSpiData(SpiInterface* spiInterface, uint8* writeBuffer, uint8* readBuffer, uint16 count);
static bool singleRead(SpiInterface* spiInterface, uint32 addr, uint8* buffer, uint32 count);
static bool pageWrite(SpiInterface* spiInterface, uint32 pageNumber, uint8* buffer);
static bool buffer1Write(SpiInterface* spiInterface, uint32 addr, uint8* buffer, uint32 count);
static bool writeBuffer1toPage(SpiInterface* spiInterface, uint32 pageNumber);

static bool isReady(SpiInterface* spiInterface);

static void loadAddrBytesWithAddr(uint8* buffer, uint32 addr);
static void loadAddrBytesWithPage(uint8* buffer, uint32 page);

void dispData(uint8* buf, uint32 count);



///////////////////////////////////////////////////
// Local data

SpiInterface spiInterfaceSoftwareFlash;
SpiInterface spiInterfacePlatformFlash;



///////////////////////////////////////////////////
// Interface functions

void serialFlashInit()
{
    spiInterfaceInit(&spiInterfacePlatformFlash, XPAR_CONFIG_FLASH_SPI_DEVICE_ID,
                     XPAR_INTC_CONFIG_FLASH_SPI_IP2INTC_IRPT_INTR,
                     XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION | XSP_CLK_ACTIVE_LOW_OPTION | XSP_CLK_PHASE_1_OPTION,
                     0);

    spiInterfaceInit(&spiInterfaceSoftwareFlash, XPAR_USER_FLASH_SPI_DEVICE_ID,
                     XPAR_INTC_USER_FLASH_SPI_IP2INTC_IRPT_INTR,
                     XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION | XSP_CLK_ACTIVE_LOW_OPTION | XSP_CLK_PHASE_1_OPTION,
                     0);
}


uint8 readStatusRegister(SpiInterface* spiInterface)
{
    uint8 writeBuffer[SPI_BUFFER_SIZE];
    uint8 readBuffer[SPI_BUFFER_SIZE];

    writeBuffer[0] = OP_STATUS_REGISTER_READ;
    writeBuffer[1] = 0x00;

    if(!sendAndReceiveSpiData(spiInterface, writeBuffer, readBuffer, 2)) return 0;

    return readBuffer[1];
}



bool eraseFlashBlock(uint32 blockNumber)
{
    ASSERT(blockNumber < FLASH_BLOCK_COUNT);

    while(!isReady(&spiInterfaceSoftwareFlash))
    { ; }

    uint8 writeBuffer[SPI_BUFFER_SIZE];
    uint8 readBuffer[SPI_BUFFER_SIZE];

    writeBuffer[0] = OP_BLOCK_ERASE;
    writeBuffer[1] = blockNumber >> 3;
    writeBuffer[2] = 0x000000ff & (blockNumber << 5);
    writeBuffer[3] = 0x00;

    return sendAndReceiveSpiData(&spiInterfaceSoftwareFlash, writeBuffer, readBuffer, 4);
}




bool readFlash(uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(buffer);

    while(!isReady(&spiInterfaceSoftwareFlash))
    { ; }

    while(count > 0)
    {
        uint32 readCount = MIN(count, MAX_SINGLE_READ_SIZE);

        if(!singleRead(&spiInterfaceSoftwareFlash, addr, buffer, readCount)) return false;
        
        buffer += readCount;
        addr   += readCount;
        count  -= readCount;
    }

    return true;
}



bool writeFlash(uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(buffer);

    while(count > 0)
    {
        uint32 page       = addr / FLASH_PAGE_SIZE;
        uint32 pageOffset = addr % FLASH_PAGE_SIZE;
        uint32 writeCount = MIN(count, FLASH_PAGE_SIZE - pageOffset);
        uint8  writeBuffer[FLASH_PAGE_SIZE];

        if(writeCount < FLASH_PAGE_SIZE)
        {
            memset(writeBuffer, 0xff, sizeof(writeBuffer));
            memcpy(&writeBuffer[pageOffset], buffer, writeCount);

            if(!pageWrite(&spiInterfaceSoftwareFlash, page, writeBuffer)) return false;
        }
        else
        {
            if(!pageWrite(&spiInterfaceSoftwareFlash, page, buffer)) return false;
        }

        buffer += writeCount;
        addr   += writeCount;
        count  -= writeCount;
    }

    return true;
}



#ifndef BOOTLOADER

bool readPlatformFlash(uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(buffer);

    while(!isReady(&spiInterfacePlatformFlash))
    { ; }

    while(count > 0)
    {
        uint32 readCount = MIN(count, MAX_SINGLE_READ_SIZE);

        if(!singleRead(&spiInterfacePlatformFlash, addr, buffer, readCount)) return false;
        
        buffer += readCount;
        addr   += readCount;
        count  -= readCount;
    }

    return true;
}



bool writePlatformFlash(uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(buffer);

    while(count > 0)
    {
        uint32 page       = addr / FLASH_PAGE_SIZE;
        uint32 pageOffset = addr % FLASH_PAGE_SIZE;
        uint32 writeCount = MIN(count, FLASH_PAGE_SIZE - pageOffset);
        uint8  writeBuffer[FLASH_PAGE_SIZE];

        if(writeCount < FLASH_PAGE_SIZE)
        {
            memset(writeBuffer, 0xff, sizeof(writeBuffer));
            memcpy(&writeBuffer[pageOffset], buffer, writeCount);

            if(!pageWrite(&spiInterfacePlatformFlash, page, writeBuffer)) return false;
        }
        else
        {
            if(!pageWrite(&spiInterfacePlatformFlash, page, buffer)) return false;
        }

        buffer += writeCount;
        addr   += writeCount;
        count  -= writeCount;
    }

    return true;
}

#endif


///////////////////////////////////////////////////
// Local functions

void dispData(uint8* buf, uint32 count)
{
    const int lineLen = 16;
    
    while(count > 0)
    {
        int i;
        for(i = 0; i < lineLen; i++)
        {
            if(i == lineLen / 2)
            {
                printf(" ");
            }
            if(i < count)
            {
                printf("%02x ", buf[i]);
            }
            else
            {
                printf("   ");
            }
        }

        printf(" ");

        for(i = 0; i < lineLen && i < count; i++)
        {
            if(i == lineLen / 2)
            {
                printf(" ");
            }
            if(i < count)
            {
                printf("%c", isprint(buf[i]) ? buf[i] : '.');
            }
            else
            {
                break;
            }
        }
        
        printf("\n");
        
        count -= MIN(count, lineLen);
        buf += lineLen;
    }
}



static bool sendAndReceiveSpiData(SpiInterface* spiInterface, uint8* writeBuffer, uint8* readBuffer, uint16 count)
{
    ASSERT(spiInterface);
    ASSERT(writeBuffer);
    ASSERT(readBuffer);
    ASSERT(count <= SPI_BUFFER_SIZE);

    uint16 readCount;

    if(spiWaitAndStartTransmit(spiInterface, writeBuffer, count, TX_TIMEOUT_us) != SPI_STATUS_TX_STARTED)  return false; // SPI Write Error
    if(spiWaitAndReadData(spiInterface, readBuffer, count, &readCount, RX_TIMEOUT_us) != SPI_STATUS_RX_OK) return false; // SPI Read Error
    if(readCount != count) return false; // SPI Read Error
    
    return true;
}



static bool singleRead(SpiInterface* spiInterface, uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(spiInterface);
    ASSERT(buffer);
    ASSERT(count <= MAX_SINGLE_READ_SIZE);
    
    uint8 writeBuffer[SPI_BUFFER_SIZE];
    uint8 readBuffer[SPI_BUFFER_SIZE];

    writeBuffer[0] = OP_CONTINUOUS_ARRAY_READ_HIGH_SPEED;
    loadAddrBytesWithAddr(&writeBuffer[1], addr);
    writeBuffer[4] = 0x00;

    if(!sendAndReceiveSpiData(spiInterface, writeBuffer, readBuffer, 5 + count)) return false;
    
    memcpy(buffer, &readBuffer[5], count);
    
    return true;
}



static bool pageWrite(SpiInterface* spiInterface, uint32 pageNumber, uint8* buffer)
{
    ASSERT(spiInterface);
    ASSERT(buffer);

    while(!isReady(spiInterface))
    { ; }

    uint32 count = FLASH_PAGE_SIZE;
    uint32 addr  = 0;
    while(count > 0)
    {
        uint32 writeCount = MIN(count, MAX_SINGLE_BUFFER_WRITE_SIZE);

        if(!buffer1Write(spiInterface, addr, buffer, writeCount)) return false;
        
        buffer += writeCount;
        addr   += writeCount;
        count  -= writeCount;
    }
    
    writeBuffer1toPage(spiInterface, pageNumber);

    return true;
}



static bool buffer1Write(SpiInterface* spiInterface, uint32 addr, uint8* buffer, uint32 count)
{
    ASSERT(spiInterface);
    ASSERT(buffer);
    ASSERT(addr + count <= FLASH_PAGE_SIZE)
    ASSERT(count <= MAX_SINGLE_BUFFER_WRITE_SIZE);
    
    uint8 writeBuffer[SPI_BUFFER_SIZE];

    writeBuffer[0] = OP_BUFFER1_WRITE;
    loadAddrBytesWithAddr(&writeBuffer[1], addr);
    memcpy(&writeBuffer[4], buffer, count);

    if(spiWaitAndStartTransmit(spiInterface, writeBuffer, 4 + count, TX_TIMEOUT_us) != SPI_STATUS_TX_STARTED)  return false; // SPI Write Error
    if(!spiWaitForReady(spiInterface, RX_TIMEOUT_us)) return false;

    return true;
}



static bool writeBuffer1toPage(SpiInterface* spiInterface, uint32 pageNumber)
{
    ASSERT(spiInterface);
    ASSERT(pageNumber < FLASH_PAGE_COUNT);
    
    uint8 writeBuffer[SPI_BUFFER_SIZE];

    writeBuffer[0] = OP_BUFFER1_TO_PAGE_PROG_WITH_ERASE;
    loadAddrBytesWithPage(&writeBuffer[1], pageNumber);

    if(spiWaitAndStartTransmit(spiInterface, writeBuffer, 4, TX_TIMEOUT_us) != SPI_STATUS_TX_STARTED)  return false; // SPI Write Error
    if(!spiWaitForReady(spiInterface, RX_TIMEOUT_us)) return false;

    return true;
}



static bool isReady(SpiInterface* spiInterface)
{
    return !!(readStatusRegister(spiInterface) & 0x80);
}



static void loadAddrBytesWithAddr(uint8* buffer, uint32 addr)
{
    ASSERT(buffer);
    ASSERT(addr < FLASH_SIZE);
    
    uint32 page   = addr / FLASH_PAGE_SIZE;
    uint32 offset = addr % FLASH_PAGE_SIZE;
    
    buffer[0] = (page >> 6) & 0x3f;
    buffer[1] = ((page << 2) & 0xfc) | ((offset >> 8) & 0x03);
    buffer[2] = offset & 0xff;
    
}



static void loadAddrBytesWithPage(uint8* buffer, uint32 page)
{
    ASSERT(buffer);
    ASSERT(page < FLASH_PAGE_COUNT);

    buffer[0] = (page >> 6) & 0x3f;
    buffer[1] = (page << 2) & 0xfc;
    buffer[2] = 0;
}



// EOF
