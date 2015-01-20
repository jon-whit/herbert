/*
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Xilinx EDK 10.1.03 EDK_K_SP3.6
 *
 * This file is a sample test application
 *
 * This application is intended to test and/or illustrate some 
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * XPS project when you run the "Generate Libraries" menu item
 * in XPS.
 *
 * Your XPS project directory is at:
 *    C:\P4\dev\david_hawks\PlateCycler\Beta\it_ddr2_test\
 */


// Located in: microblaze_0/include/xparameters.h
#include "xparameters.h"

#include "mb_interface.h"

#include "stdio.h"

#include "xutil.h"

#include <xparameters.h>




typedef unsigned long uint32;



#define PWM_CHANNEL_COUNT     24
#define LID_PWM_CHANNEL_COUNT 2


typedef struct
{
    volatile uint32 period;
    volatile uint32 enable;
    volatile uint32 error;
    volatile uint32 dutyCycle[PWM_CHANNEL_COUNT];
    volatile uint32 lidPeriod;
    volatile uint32 lidDutyCycle[LID_PWM_CHANNEL_COUNT];
} PwmRegs;


#define PWM  (*((PwmRegs*)XPAR_PWM_BASEADDR))



typedef struct
{
    volatile uint32 enable;
    volatile uint32 select;
    volatile uint32 sensor;
    volatile uint32 direction;
    volatile uint32 stepPulseWidth;
    volatile uint32 beginStep;  
} MotorRegs;

#define MOTOR (*((MotorRegs*)XPAR_MOTOR_BASEADDR))




//====================================================

void delay(int delay)
{
    int d;
    for(d = 0; d < delay; d++);
}


int main (void)
{
    
   
    volatile uint32* SPIA = XPAR_SENSOR_A_SPI_BASEADDR;
    volatile uint32* SPIB = XPAR_SENSOR_B_SPI_BASEADDR;
    
    
    volatile uint32* LED_DRIVE    = XPAR_LED_DRIVE_SPI_BASEADDR;
    volatile uint32* LED_FEEDBACK = XPAR_LED_FEEDBACK_SPI_BASEADDR;
    
    
    
    SPIA[24]         = 0x00000086;
    SPIB[24]         = 0x00000086;
    LED_DRIVE[24]    = 0x00000086;
    LED_FEEDBACK[24] = 0x00000086;
    
    
    
    
    int i = 0;
    
    while(1)
    {
        //xil_printf("%d \r\n", i);
        
        SPIA[28] = i;
        SPIB[28] = i;

        LED_DRIVE[28]    = i;
        LED_FEEDBACK[28] = i;
        
        //MOTOR.enable = i; //(i & 1) ? 0xffffffff : 0;
        //MOTOR.select = i; //(i & 1) ? 0xffffffff : 0;
        //MOTOR.sensor = i;
        //MOTOR.direction = i;
        //MOTOR.stepPulseWidth = i % 10000;
        //MOTOR.beginStep = 1;
        
        delay(500);
        
        i++;
    }    
    
    


    const int PWM_PERIOD = 100000;
    
    PWM.lidPeriod       = PWM_PERIOD;
    
    
    while(1)
    {
        int i;
        
        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[0] = i;
            delay(500);
        }
        
        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[1] = i;
            delay(500);
        }

        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[0] = PWM_PERIOD - i - 1;
            delay(500);
        }
        
        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[0] = i;
            PWM.lidDutyCycle[1] = PWM_PERIOD - i - 1;
            delay(500);
        }
        
        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[0] = i;
            PWM.lidDutyCycle[1] = PWM_PERIOD - i - 1;
            delay(500);
        }
        
        for(i = 0; i < PWM_PERIOD; i++)
        {
            PWM.lidDutyCycle[0] = PWM_PERIOD - i - 1;
            delay(500);
        }
    }

















   /*
    * Enable and initialize cache
    */
   #if XPAR_MICROBLAZE_0_USE_ICACHE
/*       microblaze_init_icache_range(0, XPAR_MICROBLAZE_0_CACHE_BYTE_SIZE); */
/*       microblaze_enable_icache(); */
   #endif

   #if XPAR_MICROBLAZE_0_USE_DCACHE
/*       microblaze_init_dcache_range(0, XPAR_MICROBLAZE_0_DCACHE_BYTE_SIZE); */
/*       microblaze_enable_dcache(); */
   #endif

/*    print("-- Entering main() --\r\n"); */

   /* Testing MPMC Memory (DDR2_SDRAM)*/
   {
       static volatile XStatus status;
      int pass = 0;
      int failed = 0;

/*       print("Starting MemoryTest for DDR2_SDRAM:\r\n"); */
/*       print("  Running 32-bit test..."); */
      status = XUtil_MemoryTest32((Xuint32*)XPAR_DDR2_SDRAM_MPMC_BASEADDR, 1024, 0xAAAA5555, XUT_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
/*          print("PASSED!\r\n"); */
          pass = 1;
      }
      else {
/*          print("FAILED!\r\n"); */
          failed = 1;
      }
/*       print("  Running 16-bit test..."); */
      status = XUtil_MemoryTest16((Xuint16*)XPAR_DDR2_SDRAM_MPMC_BASEADDR, 2048, 0xAA55, XUT_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
/*          print("PASSED!\r\n"); */
          pass = 1;
      }
      else {
/*          print("FAILED!\r\n"); */
          failed = 1;
      }
/*       print("  Running 8-bit test..."); */
      status = XUtil_MemoryTest8((Xuint8*)XPAR_DDR2_SDRAM_MPMC_BASEADDR, 4096, 0xA5, XUT_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
/*          print("PASSED!\r\n"); */
          pass = 1;
      }
      else {
/*          print("FAILED!\r\n"); */
          failed = 1;
      }
   }

   /**
    * MpmcSelfTestExample() will not be run for the memory 
    * (DDR2_SDRAM) because ECC is not supported.
    */


   /* 
    * MemoryTest routine will not be run for the memory at 
    * 0x00000000 (dlmb_cntlr)
    * because it is being used to hold a part of this application program
    */

   /*
    * Disable cache and reinitialize it so that other
    * applications can be run with no problems
    */
   #if XPAR_MICROBLAZE_0_USE_DCACHE
      microblaze_disable_dcache();
      microblaze_init_dcache_range(0, XPAR_MICROBLAZE_0_DCACHE_BYTE_SIZE);
   #endif

   #if XPAR_MICROBLAZE_0_USE_ICACHE
      microblaze_disable_icache();
      microblaze_init_icache_range(0, XPAR_MICROBLAZE_0_CACHE_BYTE_SIZE);
   #endif


/*    print("-- Exiting main() --\r\n"); */
   return 0;
}

