/////////////////////////////////////////////////////////////
//
//  AT45DB161D.h
//
//  Atmel AT45DB161D Driver
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef AT45DB161D_h
#define AT45DB161D_h

#include <types.h>


#define FLASH_PAGE_SIZE     528
#define FLASH_PAGE_COUNT    4096

#define FLASH_BLOCK_SIZE    4224
#define FLASH_BLOCK_COUNT   512

#define FLASH_SIZE          2162688




void serialFlashInit();

bool eraseFlashBlock(uint32 blockNumber);
bool readFlash(uint32 addr, uint8* buffer, uint32 count);
bool writeFlash(uint32 addr, uint8* buffer, uint32 count);



#ifndef BOOTLOADER

    bool readPlatformFlash(uint32 addr, uint8* buffer, uint32 count);
    bool writePlatformFlash(uint32 addr, uint8* buffer, uint32 count);

#endif



#endif
