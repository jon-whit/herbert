/////////////////////////////////////////////////////////////
//
//  system.h
//
//  System Controller interface

#ifndef system_h
#define system_h

#include <xparameters.h>
#include <types.h>



#define LED_ON  1
#define LED_OFF 0


#define DEBUG_LED_COUNT 8


#define FPGA_MAJOR_VER()      ((SYSTEM.fpgaVersion & 0xFFFF0000) >> 16)
#define FPGA_MINOR_VER()      (SYSTEM.fpgaVersion & 0x0000FFFF)

#define BOOTLOADER_OVERRIDE() (!(SYSTEM.dipSwitch & 0x00000001))


#define HEARTBEAT_NORMAL_PERIOD_ms 1000
#define HEARTBEAT_ERROR_PERIOD_ms  125


typedef struct
{
    volatile uint32 fpgaVersion;                // FPGA Version - 16MSbs Major Version, 16LSbs Minor Version (32 bits)
    volatile uint32 bootloaderLED;              // Bootloader LED (1 bit)
    volatile uint32 heartbeatLED;               // heartbeat LED (1 bit)
    volatile uint32 activityLED;                // activity LED (1 bit)
    volatile uint32 debugLED[DEBUG_LED_COUNT];  // Debug LEDs (1 bit each)
    volatile uint32 dipSwitch;                  // Dip Switch (4 bits)
} SystemControlRegs;


#define SYSTEM  (*((SystemControlRegs*)XPAR_SYSTEM_BASEADDR))


#endif
