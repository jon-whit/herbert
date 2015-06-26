/////////////////////////////////////////////////////////////
//
//  startup.c
//
//  System Startup
//

#include <startup.h>

#include <stdio.h>
#include <interrupt.h>
#include <assert.h>
#include <serial.h>
#include <version.h>
#include <lcd.h>
#include <timer.h>
#include <cache.h>
#include <AT45DB161D.h>
#include <os.h>
#include <system.h>
#include <motion.h>
#include <relay.h>
#include <comm.h>

///////////////////////////////////////////////////
// Interface functions

void systemStartup()
{
    disableInterrupts();

    SYSTEM.bootloaderLED = LED_OFF;
    SYSTEM.heartbeatLED  = LED_ON;
    SYSTEM.activityLED   = LED_ON;

    printf("\n\n");
    printf("-------------------------------------------\n");
    printf("   Plate Cycler App Firmware\n");
    printf("     FPGA Version: %lu.%lu\n", FPGA_MAJOR_VER(), FPGA_MINOR_VER());
    printf("     FW Version:   " FW_VER_APP "\n");
    printf("     Built:        " __DATE__ " " __TIME__ "\n");
    printf("-------------------------------------------\n\n");

    // Pre-interrupt initialization
    cacheInit();
    osInit();
    intcInit();
    timerInit();
    lcdInit();
    serialInit();
    commInit();
    motionInit();

    // Enable interrupts
    enableInterrupts();

    // Post-interrupt initialization
    serialFlashInit();
    motionInit();
    initializeAirCylinders();

    // Schedule Tasks
    scheduleTask(commProcess, NULL, false);
    printf("Comm Task Scheduled\n");
    scheduleTask(motionProcess, NULL, false);
    printf("Motion Task Scheduled\n");

    printf("System Ready.\n");

    sendLogMsg("FPGA Firmware version " FW_VER_APP " starting.");

    // Start task processor - never returns
    osStart();
}

// EOF
