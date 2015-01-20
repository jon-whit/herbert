/////////////////////////////////////////////////////////////
//
//  update.h
//
//  Plate Cycler firmware update utility
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#ifndef update_h
#define update_h

#include <types.h>


#define MAX_FILE_SIZE 10000000


bool storeFileData(uint32 fileOffset, uint8 data);


bool verifyFirmwareImage(uint32 fileSize, uint32 imageCRC);
void updateFirmware(uint32 fileSize, uint32 imageCRC);

#ifndef BOOTLOADER
	bool verifyFPGAImage(uint32 fileSize, uint32 imageCRC);
	void updateFPGA(uint32 fileSize, uint32 imageCRC);
#endif

#endif
