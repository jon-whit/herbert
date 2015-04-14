/////////////////////////////////////////////////////////////
//
//  update.c
//
//  Plate Cycler firmware update utility
//

#include <update.h>
#include <types.h>
#include <assert.h>
#include <serial.h>
#include <lcd.h>
#include <ctype.h>
#include <string.h>
#include <flash.h>
#include <reboot.h>
#include <stdio.h>
#include <timer.h>
#include <linkerSymbols.h>



///////////////////////////////////////////////////
// Constants

static uint8* const  SCRATCH_RAM      = (uint8*)&__scratch_start;
static const  uint32 SCRATCH_RAM_SIZE = (uint32)&__scratch_size;




///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    uint32 fileIndex;
    bool   imageVerified;
} UpdateData;



///////////////////////////////////////////////////
// Local function prototypes



///////////////////////////////////////////////////
// Local data

static UpdateData updataData = {0,0};



///////////////////////////////////////////////////
// Interface functions

bool storeFileData(uint32 fileOffset, uint8 data)
{
    updataData.imageVerified = false;
    
    ASSERT(fileOffset < SCRATCH_RAM_SIZE);

    if(!fileOffset)
    {
        updataData.fileIndex = 0;
    }
    else if(fileOffset < updataData.fileIndex)
    {
        // Allow stepping backwards to support
        // packet retries
        updataData.fileIndex = fileOffset;
    }
    else if(fileOffset > updataData.fileIndex)
    {
        return false;
    }

    SCRATCH_RAM[updataData.fileIndex++] = data;
    
    return true;
}



bool verifyFirmwareImage(uint32 fileSize, uint32 imageCRC)
{
    printf("Firmware Upgrade:\n");
    printf("Verifying image...\n");

    if(fileSize & 0x01)
    {
        //Must be on 16-bit boundry (no odd fileSize)
        printf("Invalid Image - odd byte count");
        return false;
    }

    if(fileSize != updataData.fileIndex)
    {
        printf("Invalid Image - Size incorrect (expected %ld, actual %ld)\n", fileSize, updataData.fileIndex);
        return false;
    }

    if(!verifyRamFirmwareImage((char*)SCRATCH_RAM, fileSize, imageCRC))
    {
        printf("Invalid Firmware Image\n");
        return false;
    }

    updataData.imageVerified = true;
    
    return true;
}



void updateFirmware(uint32 fileSize, uint32 imageCRC)
{
    if(!updataData.imageVerified)
    {
        if(!verifyFirmwareImage(fileSize, imageCRC))
        {
            printf("Invalid Firmware Image - skipping update\n");
            return;
        }
    }


    printf("Running Firmware Upgrade...\n");
    printf("Writing Flash...\n");



    if(!programFlashFirmwareImage(fileSize, SCRATCH_RAM))
    {
        printf("Write Failed\n");
        return;
    }



    printf("Verifing Flash...\n");
    if(!verifyFlashFirmwareImage())
    {
        printf("Verify Failed\n");
        return;
    }

    printf("Rebooting...\n");
    
    mdelay(1000);

    reboot();
}


#ifndef BOOTLOADER

bool verifyFPGAImage(uint32 fileSize, uint32 imageCRC)
{
    printf("FPGA Upgrade:\n");
    printf("Verifying image...\n");

    if(fileSize != updataData.fileIndex)
    {
        printf("Invalid Image - Size incorrect (expected %ld, actual %ld)\n", fileSize, updataData.fileIndex);
        return false;
    }

    if(!verifyRamFPGAImage((char*)SCRATCH_RAM, fileSize, imageCRC))
    {
        printf("Invalid FPGA Image\n");
        return false;
    }

    updataData.imageVerified = true;
    
    return true;
}



void updateFPGA(uint32 fileSize, uint32 imageCRC)
{
    if(!updataData.imageVerified)
    {
        if(!verifyFPGAImage(fileSize, imageCRC))
        {
            printf("Invalid FPGA Image - skipping update\n");
            return;
        }
    }


    printf("Running FPGA Upgrade...\n");
    printf("Writing Flash...\n");



    if(!programFlashFPGAImage(fileSize, SCRATCH_RAM))
    {
        printf("Write Failed\n");
        return;
    }



    printf("Verifing Flash...\n");
    if(!verifyFlashFPGAImage(imageCRC))
    {
        printf("Verify Failed - FPGA image is corrupt\n");
        return;
    }
    
    printf("Verified.\n");
}

#endif


// EOF
