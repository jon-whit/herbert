//-----------------------------------------------------------------------------
//!\file
//!
//!\brief Implemntation of the Hardware-Dependent Stepper Motors Module
//!
//! This file contains the implementation of the hardware-dependent portion
//! of the stepper motors module.  The functions defined here are used by
//! the hardware-independent portion of the stepper motors module to
//! interface with the hardware.
//!
//! Copyright (c) 2009 Idaho Technology Inc.
//-----------------------------------------------------------------------------

#include <assert.h>
#include <interrupt.h>
#include <stepper_hw.h>
#include <xparameters.h>
#include <timer.h>
#include <stdio.h>


//------------------------------------------------------------------------------
// Motor Hardware Definitions

typedef struct
{
    // Control Register
    //  Read: (Reading clears interrupt)
    //    bit 0 - interrupt enable
    //    bit 1 - interrupt pending
    //  Write:
    //    bit 0 - interrupt enable
    volatile uint32 control;
    
    // Enable Register
    //  Read/Write:
    //    bit 0 - Door
    //    bit 1 - Filter
    //    bit 2 - Lid
    volatile uint32 enable;
    
    // Channel Select Register
    //  Read/Write:
    //    bit 0-1 - Select signals
    volatile uint32 channelSelect;
    
    // Sensors Register
    //  Read only:
    //    bit 0 - Closed
    //    bit 1 - Open
    volatile uint32 sensors;
    
    // Direction Register
    //  Read/Write:
    //    bit 0 - Direction signal
    volatile uint32 direction;
    
    // Period Register
    //  Read/Write: (Writing starts a step sequence and will interrupt processor when complete)
    //    32 bits - Step period in ticks
    volatile uint32 period;
    
    // Pulse Width Register
    //  Read/Write:
    //    32 bits - Step pulse width in ticks
    volatile uint32 pulseWidth;
} MotorRegs_UFR;

#define MOTOR_REGS_UFR (*((MotorRegs_UFR*)XPAR_MOTOR_BASEADDR))


typedef struct
{
    // Control Register
    //  Read: (Reading clears interrupt)
    //    bit 0 - interrupt enable
    //    bit 1 - interrupt pending
    //  Write:
    //    bit 0 - interrupt enable
    volatile uint32 control;
    
    // Enable Register
    //  Read/Write:
    //    bit 0 - Door
    //    bit 1 - Filter
    //    bit 2 - Lid
    volatile uint32 enable;
    
    // Channel Select Register
    //  Read/Write:
    //    bit 0-1 - Select signals
    volatile uint32 channelSelect;
    
    // Sensors Register
    //  Read only:
    //    bit 0 - Closed
    //    bit 1 - Open
    volatile uint32 sensors;
    
    // Direction Register
    //  Read/Write:
    //    bit 0 - Direction signal
    volatile uint32 direction;
    
    // Period Register
    //  Read/Write: (Writing starts a step sequence and will interrupt processor when complete)
    //    32 bits - Step period in ticks
    volatile uint32 period;
    
    // Pulse Width Register
    //  Read/Write:
    //    32 bits - Step pulse width in ticks
    volatile uint32 pulseWidth;
} MotorRegs_DBL;

#define MOTOR_REGS_DBL (*((MotorRegs_DBL*)XPAR_MOTOR2_BASEADDR))

#define PULSE_WIDTH_us   2
#define MIN_LOW_TIME_us  1


static void stepper_hw_isr(void *callbackRef);

static void stepper_hw_isr2(void *callbackRef);



enum Motor_Hardware
{
    interrupt_enable_mask = 0x01,
    interrupt_clear_mask  = 0x02,
    
    home_sensor_mask  = 0x02,
    alt_sensor_mask   = 0x01,

    min_stepper_frequency = 10,
};

enum Motor_Hardware2
{
    interrupt_enable_mask2 = 0x01,
    interrupt_clear_mask2  = 0x02,
    
    home_sensor_mask2  = 0x02,
    alt_sensor_mask2   = 0x01,

    min_stepper_frequency2 = 10,
};




//------------------------------------------------------------------------------
void InitStepperHW_UFR(void(*step_callback)(void))
{
    ASSERT(step_callback);
    
    MOTOR_REGS_UFR.control       = interrupt_clear_mask;
    MOTOR_REGS_UFR.enable        = 0;
    MOTOR_REGS_UFR.channelSelect = 0;
    MOTOR_REGS_UFR.direction     = 0;
    MOTOR_REGS_UFR.pulseWidth    = USEC_TO_TICKS(PULSE_WIDTH_us);

    registerInterruptHandler(XPAR_INTC_MOTOR_INTERRUPT_INTR, stepper_hw_isr, step_callback);
    enableInterrupt(XPAR_INTC_MOTOR_INTERRUPT_INTR);
    
    MOTOR_REGS_UFR.control = interrupt_enable_mask | interrupt_clear_mask;
}

//------------------------------------------------------------------------------
void InitStepperHW_DBL(void(*step_callback2)(void))
{
    ASSERT(step_callback2);
    
    MOTOR_REGS_DBL.control       = interrupt_clear_mask2;
    MOTOR_REGS_DBL.enable        = 0;
    MOTOR_REGS_DBL.channelSelect = 0;
    MOTOR_REGS_DBL.direction     = 0;
    MOTOR_REGS_DBL.pulseWidth    = USEC_TO_TICKS(PULSE_WIDTH_us);

    registerInterruptHandler(XPAR_INTC_MOTOR2_INTERRUPT_INTR, stepper_hw_isr2, step_callback2);
    enableInterrupt(XPAR_INTC_MOTOR2_INTERRUPT_INTR);
    
    MOTOR_REGS_DBL.control = interrupt_enable_mask2 | interrupt_clear_mask2;
}



//------------------------------------------------------------------------------
void stepper_set_address_hw(uint32 select)
{
    MOTOR_REGS_UFR.channelSelect = select;
}

//------------------------------------------------------------------------------
void stepper_set_address_hw2(uint32 select)
{
    MOTOR_REGS_DBL.channelSelect = select;
}



//------------------------------------------------------------------------------
void stepper_set_enable_hw(uint32 enable)
{
    MOTOR_REGS_UFR.enable |= enable;
}
//------------------------------------------------------------------------------
void stepper_set_enable_hw2(uint32 enable)
{
    MOTOR_REGS_DBL.enable |= enable;
}



//------------------------------------------------------------------------------
void stepper_clear_enable_hw(uint32 enable)
{
    MOTOR_REGS_UFR.enable &= ~enable;
}
//------------------------------------------------------------------------------
void stepper_clear_enable_hw2(uint32 enable)
{
    MOTOR_REGS_DBL.enable &= ~enable;
}



//------------------------------------------------------------------------------
void stepper_start_step_hw(uint32 frequency)
{
    if(frequency < min_stepper_frequency )
    {
        frequency = min_stepper_frequency;
    }


    uint32 period = XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / frequency - 2;

    if(period < USEC_TO_TICKS(PULSE_WIDTH_us + MIN_LOW_TIME_us))
    {
        period = USEC_TO_TICKS(PULSE_WIDTH_us + MIN_LOW_TIME_us);
    }

    MOTOR_REGS_UFR.period = period;
}
//------------------------------------------------------------------------------
void stepper_start_step_hw2(uint32 frequency)
{
    if(frequency < min_stepper_frequency2 )
    {
        frequency = min_stepper_frequency2;
    }


    uint32 period = XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / frequency - 2;

    if(period < USEC_TO_TICKS(PULSE_WIDTH_us + MIN_LOW_TIME_us))
    {
        period = USEC_TO_TICKS(PULSE_WIDTH_us + MIN_LOW_TIME_us);
    }

    MOTOR_REGS_DBL.period = period;
}


//------------------------------------------------------------------------------
bool stepper_get_home_sensor_hw()
{
    return !!(MOTOR_REGS_UFR.sensors & home_sensor_mask);
}
//------------------------------------------------------------------------------
bool stepper_get_home_sensor_hw2()
{
    return !!(MOTOR_REGS_DBL.sensors & home_sensor_mask2);
}


//------------------------------------------------------------------------------
bool stepper_get_alt_sensor_hw()
{
    return !!(MOTOR_REGS_UFR.sensors & alt_sensor_mask);
}
//------------------------------------------------------------------------------
bool stepper_get_alt_sensor_hw2()
{
    return !!(MOTOR_REGS_DBL.sensors & alt_sensor_mask2);
}


//------------------------------------------------------------------------------
void stepper_clear_direction_hw()
{
    MOTOR_REGS_UFR.direction = 0;
}
//------------------------------------------------------------------------------
void stepper_clear_direction_hw2()
{
    MOTOR_REGS_DBL.direction = 0;
}

//------------------------------------------------------------------------------
void stepper_set_direction_hw()
{
    MOTOR_REGS_UFR.direction = 1;
}
//------------------------------------------------------------------------------
void stepper_set_direction_hw2()
{
    MOTOR_REGS_DBL.direction = 1;
}

//------------------------------------------------------------------------------
static void stepper_hw_isr(void *callbackRef)
{
    // Clear interrupt
    MOTOR_REGS_UFR.control = interrupt_enable_mask | interrupt_clear_mask;

    if(callbackRef)
    {
        ((void(*)())callbackRef)();
    }
}
//------------------------------------------------------------------------------
static void stepper_hw_isr2(void *callbackRef2)
{
    // Clear interrupt
    MOTOR_REGS_DBL.control = interrupt_enable_mask2 | interrupt_clear_mask2;

    if(callbackRef2)
    {
        ((void(*)())callbackRef2)();
    }
}

void DisableMotors()
{    
    MOTOR_REGS_UFR.enable = 0;
    MOTOR_REGS_DBL.enable = 0;
}
