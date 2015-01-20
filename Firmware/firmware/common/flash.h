/////////////////////////////////////////////////////////////
//
//  flash.c
//
//  Flash utility functions for the Atmel AT45DB161D SPI Flash
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef flash_h
#define flash_h

#include <types.h>
#include <AT45DB161D.h>



bool verifyRamFirmwareImage(void* image, uint32 imageSize, uint32 imageCRC);
bool verifyFlashFirmwareImage(void);
bool programFlashFirmwareImage(uint32 imageSize, void* image);
bool verifyFlashFirmwareImage(void);


#ifdef BOOTLOADER
    bool loadFlashFirmwareImage(void);
#endif


#ifndef BOOTLOADER
    
    bool verifyRamFPGAImage(void* image, uint32 imageSize, uint32 imageCRC);
    bool verifyFlashFPGAImage(uint32 imageCRC);
    bool programFlashFPGAImage(uint32 imageSize, void* image);

    #define NVRAM_DATA_BYTES    (FLASH_BLOCK_SIZE - FLASH_PAGE_SIZE)   //3696 bytes

    uint8 nvramReadByte(uint32 address);
    bool  nvramWriteByte(uint32 address, uint8 data);
    bool  nvramRead(uint32 address, uint8* data, uint32 count);
    bool  nvramWrite(uint32 address, uint8* data, uint32 count);
    bool  nvramWriteCrc(void);
    bool  nvramReset(void);

    void  nvramTest(void);
#endif



#endif
