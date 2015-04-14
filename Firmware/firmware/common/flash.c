/////////////////////////////////////////////////////////////
//
//  flash.c
//
//  Flash utility functions for the Atmel AT45DB161D SPI Flash
//
//  The Flash binary image following format:
//
//  Location  Field
//   0   - 3       Image Size - not including header and CRC
//   4   - 7       Image End - Zero from end of file to this address (bss section)
//   8   - 11      Entry point - Beginning of execution (_start1)
//   12  - 51      Exception Vectors
//   52  - n       Firmware Image
//   n+1 - n+4     32-bit CRC of whole image (0-n)
//
//  Note: Image Size, Entry Point, Image End & CRC are in network byte order
//

#include <flash.h>
#include <crc32.h>
#include <xparameters.h>
#include <types.h>
#include <string.h>
#include <processor.h>
#include <assert.h>
#include <conversion.h>
#include <stdio.h>
#include <system.h>
#include <AT45DB161D.h>
#include <timer.h>



///////////////////////////////////////////////////
// Externals from linker

extern void*  __scratch_start;
extern void*  __scratch_end;
extern uint32 __scratch_size;
extern void*  __vectors;



///////////////////////////////////////////////////
// Constants

#define MIN_SCRATCH_RAM_SIZE   0x200000  // 2MB
#define MAX_FLASH_IMAGE_SIZE   0x100000  // 1MB   ~250 Blocks
#define MAX_RAM_IMAGE_SIZE     (XPAR_DDR2_SDRAM_MPMC_HIGHADDR - XPAR_DDR2_SDRAM_MPMC_BASEADDR - MIN_SCRATCH_RAM_SIZE)


#define IMAGE_HEADER_SIZE    12
#define VECTOR_SECTION_SIZE  40
#define RESET_VECTOR_SIZE    8
#define CRC_SIZE             4


//Flash offsets are 8-bit words
#define FLASH_OFFSET_IMAGE_SIZE  0
#define FLASH_OFFSET_IMAGE_END   4
#define FLASH_OFFSET_ENTRY_POINT 8
#define FLASH_OFFSET_VECTORS     12
#define FLASH_OFFSET_IMAGE       52

#define FPGA_FLASH_IMAGE_SIZE    0x911EC   //Spartan3 XC3S1400A
#define FPGA_IMAGE_KEY           0x2000
#define FPGA_IMAGE_KEY_SIZE      2
#define FPGA_IMAGE_KEY_COUNT     16


#define NVRAM_BLOCKS               8
#define NVRAM_BLOCKS_START_INDEX   500
#define NVRAM_BLOCK_BLANK          0xFFFF
#define NVRAM_BLOCK_VALID          0xA5A5
#define NVRAM_BLOCK_INVALID        0x0000
#define NVRAM_BLOCK_START_ADDR     (NVRAM_BLOCKS_START_INDEX * FLASH_BLOCK_SIZE)


#define NVRAM_BLOCK_KEY_ADDR(b)        (NVRAM_BLOCK_START_ADDR + (b) * FLASH_BLOCK_SIZE)
#define NVRAM_BLOCK_DATA_BASE_ADDR(b)  (NVRAM_BLOCK_START_ADDR + (b) * FLASH_BLOCK_SIZE + FLASH_PAGE_SIZE)



static uint8* const  SCRATCH_RAM      = (uint8*)&__scratch_start;
static const  uint32 SCRATCH_RAM_SIZE = (uint32)&__scratch_size;



///////////////////////////////////////////////////
// Local types and macros

// Firmware Image
typedef struct
{
    uint32 imageSize;
    uint32 imageEnd;
    uint32 entryPoint;
} ImageHeader;


typedef struct
{
    uint16 blockKey;
    CRC32  crc;
} NvramFlags;


static volatile uint8*  const VECTORS = (uint8*)&__vectors;
static volatile uint8*  const SDRAM   = (uint8*)XPAR_DDR2_SDRAM_MPMC_BASEADDR;



///////////////////////////////////////////////////
// Local function prototypes

static void readImageHeaderFromRam(ImageHeader* imageHeader, void* image);
static void readImageHeaderFromFlash(ImageHeader* imageHeader);
static bool verifyImageHeader(ImageHeader* imageHeader);

#ifndef BOOTLOADER
    //Don't include NVRAM functions in bootloader
    static int  findValidNvramBlockIndex();
#endif


///////////////////////////////////////////////////
// Local data



///////////////////////////////////////////////////
// Interface functions

bool verifyRamFirmwareImage(void* image, uint32 imageSize, uint32 imageCRC)
{
    if(imageSize < IMAGE_HEADER_SIZE + VECTOR_SECTION_SIZE + CRC_SIZE)
    {
        //Minimum size too small
        printf("Invalid Image - image too small\n");
        return false;
    }

    if(imageSize & 0x00000001)
    {
        //Must be on 16-bit boundry (no odd fileSize)
        printf("Invalid Image - odd byte count (%lu)\n", imageSize);
        return false;
    }
    
    uint32 storedImageCRC;
    uint32 calculatedImageCRC = calcCRC32(INITIAL_CRC32_VALUE, (char*)image, imageSize - CRC_SIZE);
    uint32 wholeImageCRC      = calcCRC32(calculatedImageCRC, &((char*)image)[imageSize - CRC_SIZE], CRC_SIZE);
    
    memcpy(&storedImageCRC, &((char*)image)[imageSize - CRC_SIZE], sizeof(storedImageCRC));

    if(wholeImageCRC != imageCRC)
    {
        printf("Invalid Image - Bad whole image CRC (0x%08lx/0x%08lx)\n", wholeImageCRC, imageCRC);
        return false;
    }
    
    if(calculatedImageCRC != storedImageCRC)
    {
        printf("Invalid Image - Bad CRC (0x%08lx/0x%08lx)\n", calculatedImageCRC, storedImageCRC);
        return false;
    }
    
    ImageHeader imageHeader;
    readImageHeaderFromRam(&imageHeader, image);
    
    if(!verifyImageHeader(&imageHeader))
    {
        printf("Invalid Image Header\n");
        return false;
    }

    if(IMAGE_HEADER_SIZE + VECTOR_SECTION_SIZE + imageHeader.imageSize + CRC_SIZE != imageSize)
    {
        printf("Header: Invalid Image Length (expected %lu, actual %lu)\n", imageHeader.imageSize, imageSize);
        return false;
    }

    return true;
}



bool verifyFlashFirmwareImage()
{
    ImageHeader imageHeader;
    readImageHeaderFromFlash(&imageHeader);

    if(!verifyImageHeader(&imageHeader)) return false;

    CRC32 imageCrc;
    CRC32 calculatedCrc;

    readFlash(IMAGE_HEADER_SIZE + VECTOR_SECTION_SIZE + imageHeader.imageSize, (uint8*)&imageCrc, sizeof(imageCrc));
    readFlash(0, SCRATCH_RAM, IMAGE_HEADER_SIZE + VECTOR_SECTION_SIZE + imageHeader.imageSize);
    calculatedCrc = calcCRC32(INITIAL_CRC32_VALUE, (char*)SCRATCH_RAM,
                              IMAGE_HEADER_SIZE + VECTOR_SECTION_SIZE + imageHeader.imageSize);

    if(calculatedCrc != imageCrc)
    {
        printf("Image CRC incorrect\n");
        return false;
    }

    return true;
}



bool programFlashFirmwareImage(uint32 imageSize, void* image)
{
    return writeFlash(0, image, imageSize);
}



#ifdef BOOTLOADER
// Only make this function available in the bootloader.
// It destructively overwrites SDRAM with the firmware
// image in flash.
ImageHeader imageHeader;

bool loadFlashFirmwareImage()
{
    if(!verifyFlashFirmwareImage()) return false;

    readImageHeaderFromFlash(&imageHeader);

    uint8 vectors[VECTOR_SECTION_SIZE - RESET_VECTOR_SIZE];
    
    // Load vectors - skipping reset vector so a reset sends
    //                the execution back to the boot block
    readFlash(FLASH_OFFSET_VECTORS + RESET_VECTOR_SIZE, vectors, VECTOR_SECTION_SIZE - RESET_VECTOR_SIZE);


    // Load memory - text, data, etc.
    readFlash(FLASH_OFFSET_IMAGE, (uint8*)SDRAM, imageHeader.imageSize);

    disableInterrupts();
    
    memcpy(((void*)VECTORS) + RESET_VECTOR_SIZE, vectors, VECTOR_SECTION_SIZE - RESET_VECTOR_SIZE);


    // Clear zero sections - bss, etc.
    volatile uint8* mem = SDRAM + imageHeader.imageSize;

    while(mem < (uint8*)imageHeader.imageEnd)
    {
        *mem++ = 0;
    }



    printf("Starting application firmware...\n\n");
    SYSTEM.bootloaderLED = LED_OFF;

    // Jump to application image - should never return
    ((void(*)(void))imageHeader.entryPoint)();


    return true;
}
#endif



#ifndef BOOTLOADER
//Don't include NVRAM functions in bootloader

bool verifyRamFPGAImage(void* image, uint32 imageSize, uint32 imageCRC)
{
    if(imageSize != FPGA_FLASH_IMAGE_SIZE)
    {
        printf("Invalid Image - Incorrect image size\n");
        return false;
    }

    uint32 calculatedImageCRC = calcCRC32(INITIAL_CRC32_VALUE, (char*)image, imageSize);

    if(calculatedImageCRC != imageCRC)
    {
        printf("Invalid Image - Bad image CRC (0x%08lx/0x%08lx)\n", calculatedImageCRC, imageCRC);
        return false;
    }

    int i;
    for(i = 0; i < FPGA_IMAGE_KEY_COUNT; ++i)
    {
        int    offset = FPGA_FLASH_IMAGE_SIZE - FPGA_IMAGE_KEY_COUNT * FPGA_IMAGE_KEY_SIZE + i * FPGA_IMAGE_KEY_SIZE;
        uint16 key;
        memcpy(&key, &((uint8*)image)[offset], sizeof key);

        if(key != FPGA_IMAGE_KEY)
        {
            printf("Invalid Image - Bad image key 0x%04x at 0x%08x\n", key, offset);
        }
    }

    return true;
}



bool verifyFlashFPGAImage(uint32 imageCRC)
{
    readPlatformFlash(0, SCRATCH_RAM, FPGA_FLASH_IMAGE_SIZE);

    CRC32 calculatedCrc = calcCRC32(INITIAL_CRC32_VALUE, (char*)SCRATCH_RAM, FPGA_FLASH_IMAGE_SIZE);

    if(calculatedCrc != imageCRC)
    {
        printf("Image CRC incorrect\n");
        return false;
    }

    return true;
}



bool programFlashFPGAImage(uint32 imageSize, void* image)
{
    return writePlatformFlash(0, image, imageSize);
}



uint8 nvramReadByte(uint32 address)
{
    uint8 data;
    
    nvramRead(address, &data, sizeof(data));
    
    return data;
}



bool nvramWriteByte(uint32 address, uint8 data)
{
    return nvramWrite(address, &data, sizeof(data));
}



bool nvramRead(uint32 address, uint8* data, uint32 count)
{
    ASSERT(address + count < NVRAM_DATA_BYTES);

    int nvramBlock = findValidNvramBlockIndex();

    if(nvramBlock >= 0)
    {
        uint8 blockData[NVRAM_DATA_BYTES];

        if(readFlash(NVRAM_BLOCK_DATA_BASE_ADDR(nvramBlock), blockData, NVRAM_DATA_BYTES))
        {
            NvramFlags nvramFlags;
            CRC32      calculatedCrc = calcCRC32(INITIAL_CRC32_VALUE, blockData, NVRAM_DATA_BYTES);

            if(readFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&nvramFlags, sizeof(nvramFlags)))
            {
                if(nvramFlags.crc == calculatedCrc)
                {
                    memcpy(data, blockData + address, count);
                    return true;
                }
            }
        }
    }

    memset(data, 0xff, count);
    return false;
}



bool nvramWrite(uint32 address, uint8* data, uint32 count)
{
    ASSERT(address + count < NVRAM_DATA_BYTES);


    NvramFlags nvramFlags;
    uint8      blockData[NVRAM_DATA_BYTES];
    int        nvramBlock = findValidNvramBlockIndex();

    if(nvramBlock < 0)
    {
        // No current block found - start one at index 0
        eraseFlashBlock(NVRAM_BLOCKS_START_INDEX);

        // Write the data
        if(!writeFlash(NVRAM_BLOCK_DATA_BASE_ADDR(0) + address, data, count)) return false;

        // Calculate the CRC
        readFlash(NVRAM_BLOCK_DATA_BASE_ADDR(0), blockData, NVRAM_DATA_BYTES);
        nvramFlags.crc = calcCRC32(INITIAL_CRC32_VALUE, blockData, NVRAM_DATA_BYTES);

        // Validate the block
        nvramFlags.blockKey = NVRAM_BLOCK_VALID;
        if(!writeFlash(NVRAM_BLOCK_KEY_ADDR(0), (uint8*)&nvramFlags, sizeof(nvramFlags))) return false;
    }
    else
    {
        // Read the data to determine if we need to write anything
        if(!readFlash(NVRAM_BLOCK_DATA_BASE_ADDR(nvramBlock), blockData, NVRAM_DATA_BYTES)) return false;
        if(!readFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&nvramFlags, sizeof(nvramFlags))) return false;
        CRC32 calculatedCrc = calcCRC32(INITIAL_CRC32_VALUE, blockData, NVRAM_DATA_BYTES);
        
        if(memcmp(&blockData[address], data, count) == 0 &&
           nvramFlags.crc == calculatedCrc)
        {
            // Data the same, no need to write
            printf("NVRAM Write - Data the same, skip write\n");
            return true;
        }
        else
        {
            // Must copy current block to new block


            // Set new block 1 above current block
            int newNvramBlock = (nvramBlock + 1) % NVRAM_BLOCKS;


            // Clear key for new block
            nvramFlags.blockKey = NVRAM_BLOCK_BLANK;
            if(!writeFlash(NVRAM_BLOCK_KEY_ADDR(newNvramBlock), (uint8*)&nvramFlags.blockKey, sizeof(nvramFlags.blockKey))) return false;


            // Update block data
            memcpy(&blockData[address], data, count);


            // Program updated data to new block
            if(!writeFlash(NVRAM_BLOCK_DATA_BASE_ADDR(newNvramBlock), blockData, NVRAM_DATA_BYTES)) return false;


            // Calculate the CRC
            nvramFlags.crc = calcCRC32(INITIAL_CRC32_VALUE, blockData, NVRAM_DATA_BYTES);

            // Validate new block
            nvramFlags.blockKey = NVRAM_BLOCK_VALID;
            if(!writeFlash(NVRAM_BLOCK_KEY_ADDR(newNvramBlock), (uint8*)&nvramFlags, sizeof(nvramFlags))) return false;


            // Invalidate old block
            nvramFlags.blockKey = NVRAM_BLOCK_INVALID;
            if(!writeFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&nvramFlags.blockKey, sizeof(nvramFlags.blockKey))) return false;
            
            
            return true;
        }
    }


    return false;
}


bool nvramWriteCrc()
{
    CRC32      calculatedCrc;
    NvramFlags nvramFlags;
    uint8      blockData[NVRAM_DATA_BYTES];        
    int        nvramBlock = findValidNvramBlockIndex();

    if(nvramBlock < 0)
    {
        return true;
    }

    if(readFlash(NVRAM_BLOCK_DATA_BASE_ADDR(nvramBlock), blockData, NVRAM_DATA_BYTES) &&
       readFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&nvramFlags, sizeof(nvramFlags)))
    {
        calculatedCrc = calcCRC32(INITIAL_CRC32_VALUE, blockData, NVRAM_DATA_BYTES);

        if(nvramFlags.crc != calculatedCrc)
        {
            nvramFlags.crc = calculatedCrc;
            return writeFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&nvramFlags, sizeof(nvramFlags));
        }
    }

    return false;
}


bool nvramReset()
{
    int blockNumber;
    
    for(blockNumber = 0; blockNumber < NVRAM_BLOCKS; blockNumber++)
    {
        bool rc = eraseFlashBlock(NVRAM_BLOCKS_START_INDEX + blockNumber);
        if(!rc) return false;
    }

    return true;
}



void nvramTest()
{
    uint64 startTicks;
    uint64 endTicks;
    uint8  byte;
    
    mdelay(1000);

    printf("NVRAM Tests...\n");
    
    startTicks = getSystemTicks();
    nvramReset();
    endTicks = getSystemTicks();
    
    printf("Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));
    
    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x00);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x11);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x22);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x33);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x44);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x55);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x66);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x77);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    nvramWriteByte(0, 0x88);
    endTicks = getSystemTicks();
    printf("Byte Write: Elapsed time %llu us\n", TICKS_TO_USEC(endTicks - startTicks));

    startTicks = getSystemTicks();
    byte = nvramReadByte(0);
    endTicks = getSystemTicks();
    printf("Byte Read: 0x%02x - Elapsed time %llu us\n", byte, TICKS_TO_USEC(endTicks - startTicks));


    printf("NVRAM Tests complete.\n");
}


#endif //#ifndef BOOTLOADER


///////////////////////////////////////////////////
// Local functions

static void readImageHeaderFromRam(ImageHeader* imageHeader, void* image)
{
    memcpy((uint8*)&imageHeader->imageSize, image + FLASH_OFFSET_IMAGE_SIZE, sizeof(imageHeader->imageSize));
    memcpy((uint8*)&imageHeader->imageEnd, image + FLASH_OFFSET_IMAGE_END, sizeof(imageHeader->imageEnd));
    memcpy((uint8*)&imageHeader->entryPoint, image + FLASH_OFFSET_ENTRY_POINT, sizeof(imageHeader->entryPoint));
}



static void readImageHeaderFromFlash(ImageHeader* imageHeader)
{
    readFlash(FLASH_OFFSET_IMAGE_SIZE, (uint8*)&imageHeader->imageSize, sizeof(imageHeader->imageSize));
    readFlash(FLASH_OFFSET_IMAGE_END, (uint8*)&imageHeader->imageEnd, sizeof(imageHeader->imageEnd));
    readFlash(FLASH_OFFSET_ENTRY_POINT, (uint8*)&imageHeader->entryPoint, sizeof(imageHeader->entryPoint));
}



static bool verifyImageHeader(ImageHeader* imageHeader)
{
    printf("Header: Size  0x%08lx\n", imageHeader->imageSize);
    printf("        End   0x%08lx\n", imageHeader->imageEnd);
    printf("        Entry 0x%08lx\n", imageHeader->entryPoint);
    
    
    if(imageHeader->imageSize  > MAX_FLASH_IMAGE_SIZE)
    {
        printf("Header: Image size too large\n");
        return false;
    }
    if(imageHeader->imageSize  & 0x00000001)
    {
        printf("Header: Image not 16-bit aligned (%lu)\n", imageHeader->imageSize);
        return false;  //Must be 16-bit aligned
    }
    if(imageHeader->imageEnd   < XPAR_DDR2_SDRAM_MPMC_BASEADDR)
    {
        printf("Header: Image end address lower than SDRAM Base Addr (0x%08lx)\n", imageHeader->imageEnd);
        return false;
    }
    if(imageHeader->imageEnd   > XPAR_DDR2_SDRAM_MPMC_BASEADDR + MAX_RAM_IMAGE_SIZE)
    {
        printf("Header: Image end address beyond end of SDRAM (0x%08lx)\n", imageHeader->imageEnd);
        return false;
    }
    if(imageHeader->entryPoint < XPAR_DDR2_SDRAM_MPMC_BASEADDR)
    {
        printf("Header: Image entry point lower than SDRAM Base Addr (0x%08lx)\n", imageHeader->entryPoint);
        return false;
    }
    if(imageHeader->entryPoint > XPAR_DDR2_SDRAM_MPMC_BASEADDR + MAX_RAM_IMAGE_SIZE)
    {
        printf("Header: Image entry point beyond end of SDRAM (0x%08lx)\n", imageHeader->entryPoint);
        return false;
    }

    return true;
}



#ifndef BOOTLOADER
//Don't include NVRAM functions in bootloader

static int findValidNvramBlockIndex()
{
    uint32 nvramBlock;

    for(nvramBlock = 0; nvramBlock < NVRAM_BLOCKS; nvramBlock++)
    {
        uint16 buf;
        
        readFlash(NVRAM_BLOCK_KEY_ADDR(nvramBlock), (uint8*)&buf, sizeof(buf));
        if(buf == NVRAM_BLOCK_VALID)
        {
            // Current valid block found
            return nvramBlock;
        }
    }
    
    printf("No valid NVRAM Blocks found\n");

    return -1;
}



#endif


// EOF
