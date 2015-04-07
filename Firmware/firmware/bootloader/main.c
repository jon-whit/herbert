/////////////////////////////////////////////////////////////
//
//  main.c
//
//  Bootloader main


#include <stdio.h>
#include <xutil.h>

#include <types.h>
#include <version.h>
#include <processor.h>
#include <timer.h>
#include <flash.h>
#include <serial.h>
#include <comm.h>
#include <interrupt.h>
#include <vectors.h>
#include <mb_interface.h>
#include <AT45DB161D.h>
#include <system.h>



int main (void)
{
    microblaze_disable_icache();
    microblaze_disable_dcache();

    disableInterrupts();

    SYSTEM.bootloaderLED = LED_ON;

    printf("\n\n");
    printf("===========================================\n");
    printf("   Herbert Bootloader\n");
    printf("     FPGA Version: %lu.%lu\n", FPGA_MAJOR_VER(), FPGA_MINOR_VER());
    printf("     FW Version:   " FW_VER_BOOT "\n");
    printf("     Built:        " __DATE__ " " __TIME__ "\n");
    printf("===========================================\n\n");

    loadExceptionVectors();
    intcInit();
    timerInit();
    serialInit();

    enableInterrupts();
    serialFlashInit();


    uint32 heartbeatPeriod;

    if(!BOOTLOADER_OVERRIDE())
    {
        printf("Loading application firmware...\n");

        // Run Bootloader
        loadFlashFirmwareImage();
        
        // if this function returns, the firmware
        // image failed to load.
        
        enableInterrupts();
        printf("Firmware Load failed\n");
        
        heartbeatPeriod = MSEC_TO_TICKS(HEARTBEAT_ERROR_PERIOD_ms);
    }
    else
    {
        // DIP SW 0 was set, override Bootloader
        printf("Firmware Load Override\n");
        
        heartbeatPeriod = MSEC_TO_TICKS(HEARTBEAT_NORMAL_PERIOD_ms);
    }


    Timer heartbeatTimer;
    startTimer(&heartbeatTimer, heartbeatPeriod);

    while(1)
    {
        // Main loop processing
        serialProcess();
        commProcess();

        if(timerExpired(&heartbeatTimer))
        {
            SYSTEM.bootloaderLED = ~SYSTEM.bootloaderLED;
            startTimer(&heartbeatTimer, heartbeatPeriod);
        }
    }

    return 0;
}




