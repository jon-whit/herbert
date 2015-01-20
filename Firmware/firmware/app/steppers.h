//-----------------------------------------------------------------------------
//!\file
//!
//!\brief Stepper Motors Module Interface
//!
//! This file contains the interface to the stepper motors module.  The
//! stepper motors module controls the lid, door, and filter wheel stepper
//! motors of the PlateCycler.
//!
//! Copyright (c) 2009 Idaho Technology Inc.
//-----------------------------------------------------------------------------

#ifndef steppers_h
#define steppers_h

#include <limits.h>
#include <types.h>

typedef enum 
{ 
    stepperU, 
    stepperF, 
    stepperR, 
    //stepperD,
    //stepperB,
    //stepperL,
    numSteppers 
} StepperMotor;

enum StepperConstants
{
    stepperHome = INT_MAX, /*!< This value is reserved for home requests. */
  
    // These values need to be tested to see what frequencies work best  
    defaultFastFrequency    = 2000,
    defaultSlowFrequency    = 100,
    defaultRampSteps        = 10,
};

typedef enum
{
    stepperFastFrequency,
    stepperSlowFrequency,
    stepperRampSteps,
    stepperMaxSteps 
} StepperParameter;




void stepper_init( void );
void stepper_abort( void );
bool stepper_move_to_position( StepperMotor stepper, int position );
bool stepper_move_relative( StepperMotor stepper, int steps );
bool stepper_enable( StepperMotor stepper, bool enable );
bool stepper_busy_wait( uint32 ticks );
bool stepper_busy( void );
void stepper_reset_home_pin_location( void );
int  stepper_get_home_pin_location( StepperMotor stepper );
bool stepper_set_parameter( StepperMotor stepper, StepperParameter param, int value );
void stepper_set_home_sensor_make_hook( StepperMotor stepper, bool ( *hook_func )( int ) );
void stepper_set_home_sensor_break_hook( StepperMotor stepper, bool ( *hook_func )( int ) );
void stepper_set_alt_sensor_make_hook( StepperMotor stepper, bool ( *hook_func )( int ) );
void stepper_set_alt_sensor_break_hook( StepperMotor stepper, bool ( *hook_func )( int ) );
bool stepper_is_at_home_position( StepperMotor stepper );
bool stepper_is_at_alt_position( StepperMotor stepper );
int  get_stepper_position( StepperMotor stepper );


#endif //...#ifndef steppers_h
