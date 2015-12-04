//-----------------------------------------------------------------------------
//!\file
//!
//!\brief Stepper Motors Module
//!
//! This file contains the implementation of the stepper motors module.
//! The stepper motors module controls the steppers motors.  This module
//! requires a hardware-dependent module that implements the hardware
//! interface.
//!
//-----------------------------------------------------------------------------

#include <timer.h>
#include <steppers.h>
#include <stepper_hw.h>
#include <processor.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    int position;
    int home_position;
    int home_pin_location;
    int max_steps;
    int slow_freq;
    int fast_freq;
    int current_freq;
    int ramp_steps;
    int fault;
    bool disable_after_move_at_home;
    bool disable_after_move_at_alt;
    bool disable_after_move_between;
    bool adjust_home_position;
    uint32 hw_address;
    uint32 hw_enable_mask;
    bool isDBL;
    bool ( *home_sensor_make_hook )( int );
    bool ( *home_sensor_break_hook )( int );
    bool ( *alt_sensor_make_hook )( int );
    bool ( *alt_sensor_break_hook )( int );
} stepper_instance;

struct
{
    stepper_instance steppers[ numSteppers ];
    stepper_instance *moving_stepper;
    int target_position;
    int remaining_steps;
    int up_steps;
    int down_steps;
    int steps_taken;
    int freq_adjust;
    int increment;
} stepper_data;

enum stepper_hardware_constants
{
    max_lid_steps            = 20000,
    max_door_steps           = 35000,
    max_filter_steps         = 35000,

    lid_home_pin_location    = 50,
    door_home_pin_location   = 20,
    filter_home_pin_location = 0,

    lid_home_position        = 0,
    door_home_position       = 0,
    filter_home_position     = 0,

    lid_hw_enable_mask       = 0x04,
    lid_hw_address           = 0x01,
    filter_hw_enable_mask    = 0x02,
    filter_hw_address        = 0x03,
    door_hw_enable_mask      = 0x01,
    door_hw_address          = 0x00,

    uSensorAddressMask       = 0x01,
    fSensorAddressMask       = 0x02,
    rSensorAddressMask       = 0x03,

    dSensorAddressMask       = 0x01,
    bSensorAddressMask       = 0x02,
    lSensorAddressMask       = 0x03,

};

static void stepper_init_instance( stepper_instance *stepper, StepperMotor id );
static void stepper_motor_callback( void );
static int  stepper_find_ramp_freq_adjust( stepper_instance *stepper );
static void stepper_disable( stepper_instance * moving_stepper );
static bool isDBLMotor(StepperMotor stepper);

//------------------------------------------------------------------------------
//! Initialize the stepper motor module.
//
//! This function initializes the stepper motor module.
//!
void stepper_init( void )
{
    int stepper_index;

    stepper_abort();

    for( stepper_index = 0; stepper_index < numSteppers; ++stepper_index )
    {
        stepper_init_instance( &stepper_data.steppers[ stepper_index ],
                                stepper_index );
    }

    init_stepper_hw_UFR(stepper_motor_callback);
    init_stepper_hw_DBL(stepper_motor_callback);
}

//------------------------------------------------------------------------------
void stepper_abort( void )
{
    CRData crdata = enterCriticalRegion();
    {
        // If not ramping down already, start.
        if( stepper_data.moving_stepper)
        {
            if(!stepper_data.remaining_steps)
            {
                stepper_disable( stepper_data.moving_stepper );
                stepper_data.moving_stepper = NULL;
            }
            else if(stepper_data.up_steps ||
                    stepper_data.remaining_steps >= stepper_data.down_steps)
            {
                int new_remaining_steps = stepper_data.moving_stepper->ramp_steps - stepper_data.up_steps;
                int new_target_position = stepper_data.moving_stepper->position +
                                          stepper_data.increment * new_remaining_steps;
                
                if(((stepper_data.target_position - new_target_position) * stepper_data.increment) < 0)
                {
                    // New target position would move beyond the old target temp
                    new_remaining_steps = abs(stepper_data.target_position - stepper_data.moving_stepper->position);
                    new_target_position = stepper_data.target_position;
                }

                stepper_data.remaining_steps = new_remaining_steps;
                stepper_data.down_steps      = new_remaining_steps;
                stepper_data.target_position = new_target_position;

                stepper_data.up_steps = 0;
            }
        }
    }
    exitCriticalRegion(crdata);
}

//------------------------------------------------------------------------------
//! Function to set the various stepper parameters.
//
//! This function sets the \a param of the \a stepper to \a value.
//!
bool stepper_set_parameter( StepperMotor stepper, StepperParameter param, int value )
{
    if( stepper < numSteppers )
    {
        switch( param )
        {
        case stepperFastFrequency:
            stepper_data.steppers[ stepper ].fast_freq = value;
            break;
        case stepperSlowFrequency:
            stepper_data.steppers[ stepper ].slow_freq = value;
            break;
        case stepperRampSteps:
            stepper_data.steppers[ stepper ].ramp_steps = value;
            break;
        case stepperMaxSteps:
            stepper_data.steppers[ stepper ].max_steps = value;
            break;
        default:
            ASSERT( 0 );
            return false;
        }
        return true;
    }
    ASSERT( 0 );
    return false;
}

//------------------------------------------------------------------------------
bool stepper_busy_wait( uint32 ticks )
{
    Timer wait_timer;

    startTimer( &wait_timer, ticks );

    while( stepper_busy() )
    {
        if( timerExpired( &wait_timer ) )
            return false;
    }
    return true;
}

//------------------------------------------------------------------------------
bool stepper_busy( void )
{
    return stepper_data.moving_stepper != NULL;
}

//------------------------------------------------------------------------------
void stepper_reset_home_pin_location( void )
{
    // Called at ISR context
    
    if( stepper_data.moving_stepper &&
        stepper_data.moving_stepper->position != stepper_data.moving_stepper->home_pin_location )
    {
        printf("Adjusting stepper home pin position from %d to %d.\n",
               stepper_data.moving_stepper->position,
               stepper_data.moving_stepper->home_pin_location);
        stepper_data.moving_stepper->position = stepper_data.moving_stepper->home_pin_location;
    }
}

//------------------------------------------------------------------------------
int stepper_get_home_pin_location( StepperMotor stepper )
{
    ASSERT( stepper < numSteppers );
    return stepper_data.steppers[ stepper ].home_pin_location;
}

//------------------------------------------------------------------------------
bool stepper_enable( StepperMotor stepper, bool enable )
{
    ASSERT( stepper < numSteppers );
    if( stepper_data.moving_stepper != &stepper_data.steppers[ stepper ] )
    {
        if( enable )
            if(isDBLMotor(stepper))
            {
                stepper_set_enable_hw_DBL( stepper_data.steppers[ stepper ].hw_enable_mask );
            }
            else
            {
                stepper_set_enable_hw_UFR( stepper_data.steppers[ stepper ].hw_enable_mask );
            }
        else
            if(isDBLMotor(stepper))
            {
                stepper_clear_enable_hw_DBL( stepper_data.steppers[ stepper ].hw_enable_mask );
            }
            else
            {
                stepper_clear_enable_hw_UFR( stepper_data.steppers[ stepper ].hw_enable_mask );
            }

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool stepper_move_relative( StepperMotor stepper, int steps )
{
    ASSERT( stepper < numSteppers );
    return stepper_move_to_position( stepper,
                                     stepper_data.steppers[ stepper ].position + steps );
}

//------------------------------------------------------------------------------
bool stepper_move_to_position( StepperMotor stepper, int position )
{
    ASSERT( stepper < numSteppers );
    if( stepper_data.moving_stepper )
        return false;

    if(isDBLMotor(stepper))
    {
        stepper_set_address_hw_DBL( stepper_data.steppers[ stepper ].hw_address );
    }
    else
    {
        stepper_set_address_hw_UFR( stepper_data.steppers[ stepper ].hw_address );
    }
    
    // if( ( stepper_data.steppers[ stepper ].position == position ) ||
    //     ( stepper_get_home_sensor_hw_UFR() && position == stepperHome ) )
    // {
    //     return true; // No need to move, we are already there.
    // }

    stepper_data.moving_stepper  = &stepper_data.steppers[ stepper ];
    stepper_data.target_position = position;
    stepper_data.steps_taken     = 0;
    stepper_data.freq_adjust     = stepper_find_ramp_freq_adjust( stepper_data.moving_stepper );
    
    if(isDBLMotor(stepper))
    {
        stepper_set_enable_hw_DBL( stepper_data.moving_stepper->hw_enable_mask );
    }
    else
    {
        stepper_set_enable_hw_UFR( stepper_data.moving_stepper->hw_enable_mask );
    }

    if( position == stepperHome )
    {
        stepper_data.increment = -1;
        if(isDBLMotor(stepper))
        {
            stepper_clear_direction_hw_DBL( stepper_data.moving_stepper );
        }
        else
        {
            stepper_clear_direction_hw_UFR( stepper_data.moving_stepper );
        }

        stepper_data.down_steps = stepper_data.remaining_steps = 0;
        stepper_data.up_steps = stepper_data.moving_stepper->ramp_steps;
    }
    else
    {
        stepper_data.remaining_steps =
            abs( stepper_data.moving_stepper->position - position );

        if( position < stepper_data.moving_stepper->position )
        {
            stepper_data.increment = -1;
            if(isDBLMotor(stepper))
            {
                stepper_clear_direction_hw_DBL( stepper_data.moving_stepper );
            }
            else
            {
                stepper_clear_direction_hw_UFR( stepper_data.moving_stepper );
            }
        }
        else
        {
            stepper_data.increment = 1;
            if(isDBLMotor(stepper))
            {
                stepper_set_direction_hw_DBL( stepper_data.moving_stepper );
            }
            else
            {
                stepper_set_direction_hw_UFR( stepper_data.moving_stepper );
            }
        }

        if ( stepper_data.remaining_steps >=
             2 * stepper_data.moving_stepper->ramp_steps )
        {
            stepper_data.up_steps =
                stepper_data.down_steps = stepper_data.moving_stepper->ramp_steps;
        }
        else
        {
            stepper_data.up_steps   = stepper_data.remaining_steps / 2;
            stepper_data.down_steps = stepper_data.remaining_steps - stepper_data.up_steps;
        }
    }

    stepper_data.moving_stepper->current_freq = stepper_data.moving_stepper->slow_freq;

    if(isDBLMotor(stepper))
    {
        stepper_start_step_hw_DBL( stepper_data.moving_stepper->current_freq );
    }
    else
    {
        stepper_start_step_hw_UFR( stepper_data.moving_stepper->current_freq );
    }

    return true;
}


//------------------------------------------------------------------------------
void stepper_set_home_sensor_make_hook( StepperMotor stepper, bool ( *hook_func )( int ) )
{
    ASSERT( stepper < numSteppers );
    stepper_data.steppers[ stepper ].home_sensor_make_hook = hook_func;
}


//------------------------------------------------------------------------------
void stepper_set_home_sensor_break_hook( StepperMotor stepper, bool ( *hook_func )( int ) )
{
    ASSERT( stepper < numSteppers );
    stepper_data.steppers[ stepper ].home_sensor_break_hook = hook_func;
}


//------------------------------------------------------------------------------
void stepper_set_alt_sensor_make_hook( StepperMotor stepper, bool ( *hook_func )( int ) )
{
    ASSERT( stepper < numSteppers );
    stepper_data.steppers[ stepper ].alt_sensor_make_hook = hook_func;
}


//------------------------------------------------------------------------------
void stepper_set_alt_sensor_break_hook( StepperMotor stepper, bool ( *hook_func )( int ) )
{
    ASSERT( stepper < numSteppers );
    stepper_data.steppers[ stepper ].alt_sensor_break_hook = hook_func;
}


//------------------------------------------------------------------------------
bool stepper_is_at_home_position( StepperMotor stepper )
{
    ASSERT( stepper < numSteppers );
    if( !stepper_busy() )
    {
        if(isDBLMotor(stepper))
        {
            stepper_set_address_hw_DBL( stepper_data.steppers[ stepper ].hw_address );
        }
        else
        {
            stepper_set_address_hw_UFR( stepper_data.steppers[ stepper ].hw_address );
        }
    }
    bool isAtHomePosition = false;
    if(isDBLMotor(stepper))
    {
        isAtHomePosition = stepper_get_home_sensor_hw_DBL();
    }
    else
    {
        isAtHomePosition = stepper_get_home_sensor_hw_UFR();
    }
    return isAtHomePosition;
}


//------------------------------------------------------------------------------
bool stepper_is_at_alt_position( StepperMotor stepper )
{
    ASSERT( stepper < numSteppers );
    if( !stepper_busy() )
    {
        if(isDBLMotor(stepper))
        {
            stepper_set_address_hw_DBL( stepper_data.steppers[ stepper ].hw_address );
        }
        else
        {
            stepper_set_address_hw_UFR( stepper_data.steppers[ stepper ].hw_address );
        }
    }
    bool isAtAltPosition = false;
    if(isDBLMotor(stepper))
    {
        isAtAltPosition = stepper_get_alt_sensor_hw_DBL();
    }
    else
    {
        isAtAltPosition = stepper_get_alt_sensor_hw_UFR();
    }
    return isAtAltPosition;
}

//------------------------------------------------------------------------------
int get_stepper_position( StepperMotor stepper )
{
    ASSERT( stepper < numSteppers );
    return stepper_data.steppers[ stepper ].position;
}

bool isSensorBeamBroken(StepperMotor stepper)
{
    switch(stepper)
    {
    case stepperU: stepper_set_address_hw_UFR(uSensorAddressMask); return stepper_get_alt_sensor_hw_UFR(); break;
    case stepperF: stepper_set_address_hw_UFR(fSensorAddressMask); return stepper_get_alt_sensor_hw_UFR(); break;
    case stepperR: stepper_set_address_hw_UFR(rSensorAddressMask); return stepper_get_alt_sensor_hw_UFR(); break;
    case stepperD: stepper_set_address_hw_DBL(dSensorAddressMask); return stepper_get_alt_sensor_hw_DBL(); break;
    case stepperB: stepper_set_address_hw_DBL(bSensorAddressMask); return stepper_get_alt_sensor_hw_DBL(); break;
    case stepperL: stepper_set_address_hw_DBL(lSensorAddressMask); return stepper_get_alt_sensor_hw_DBL(); break;
    default:                                                                                               break;
    }
    return 0;
}

//------------------------------------------------------------------------------
//! Stepper motor ISR callback.
//
//! This function is the interrupt service routine for the stepper motors.  The
//! stepper motor module uses a timer to control the stepper motors.  This
//! function is called when the timer expires.
//!
//! The interrupt service routine is responsible for tracking the location of
//! the steppers and moving the steppers to their target locations.  The home
//! sensors are monitored here.  This function is responsible for ramping
//! the step frequency up and down during each move to avoid misstepping
//! and jarring motion on the steppers.
//
static void stepper_motor_callback( void )
{
    bool abort_move = false;
    stepper_instance* moving_stepper = stepper_data.moving_stepper;

    if(!moving_stepper)
    {
        return;
    }

    moving_stepper->position += stepper_data.increment;
    ++stepper_data.steps_taken;

    if( (stepper_get_home_sensor_hw_UFR() && !moving_stepper->isDBL) ||
        (stepper_get_home_sensor_hw_DBL() && moving_stepper->isDBL))
    {
        if( moving_stepper->home_sensor_make_hook )
        {
            abort_move = !moving_stepper->home_sensor_make_hook( moving_stepper->position );
        }
    }
    else
    {
        if( moving_stepper->home_sensor_break_hook )
        {
            abort_move = !moving_stepper->home_sensor_break_hook( moving_stepper->position );
        }
    }

    if( (stepper_get_alt_sensor_hw_UFR() && !moving_stepper->isDBL) ||
        (stepper_get_alt_sensor_hw_DBL() && moving_stepper->isDBL))
    {
        if( moving_stepper->alt_sensor_make_hook )
        {
            abort_move = !moving_stepper->alt_sensor_make_hook( moving_stepper->position );
        }
    }
    else
    {
        if( moving_stepper->alt_sensor_break_hook)
        {
            abort_move = !moving_stepper->alt_sensor_break_hook( moving_stepper->position );
        }
    }

    if( stepper_data.target_position == stepperHome && !abort_move )
    {
        if( stepper_data.steps_taken > moving_stepper->max_steps )
        {
            moving_stepper->fault = 1;
            abort_move = true;
        }
        else if( (stepper_get_home_sensor_hw_UFR() && !moving_stepper->isDBL) ||
                 (stepper_get_home_sensor_hw_DBL() && moving_stepper->isDBL))
        {
            moving_stepper->fault = 0;
            moving_stepper->position = moving_stepper->home_pin_location;

            // Step past home enough to ramp the frequency down.
            stepper_data.remaining_steps =
                stepper_data.down_steps =
                moving_stepper->ramp_steps - stepper_data.up_steps;

            stepper_data.target_position =
                moving_stepper->home_pin_location - stepper_data.remaining_steps;
        }
    }

    if( abort_move ||
       ( stepper_data.target_position != stepperHome &&
        ( ( stepper_data.target_position - moving_stepper->position ) * stepper_data.increment ) <= 0 ) )
    {
        stepper_disable( moving_stepper );
        stepper_data.moving_stepper = NULL;
    }
    else
    {
        // Ramp the stepper frequency up or down.
        if( stepper_data.remaining_steps )
            --stepper_data.remaining_steps;

        if( stepper_data.up_steps )
            --stepper_data.up_steps;

        if( stepper_data.up_steps )
        {
            stepper_data.moving_stepper->current_freq += stepper_data.freq_adjust;
        }
        else if( stepper_data.down_steps && stepper_data.remaining_steps <=
                 stepper_data.down_steps )
        {
            stepper_data.moving_stepper->current_freq -= stepper_data.freq_adjust;
        }
        
        if( stepper_data.moving_stepper->current_freq <
            stepper_data.moving_stepper->slow_freq )
        {
            stepper_data.moving_stepper->current_freq =
                stepper_data.moving_stepper->slow_freq;
        }

        if(moving_stepper->isDBL)
        {
            stepper_start_step_hw_DBL( stepper_data.moving_stepper->current_freq );
        }
        else
        {
            stepper_start_step_hw_UFR( stepper_data.moving_stepper->current_freq );

        }
    }
}



//------------------------------------------------------------------------------
static int stepper_find_ramp_freq_adjust( stepper_instance *stepper )
{
    ASSERT(stepper);
    return (stepper->fast_freq - stepper->slow_freq) / stepper->ramp_steps;
}



//------------------------------------------------------------------------------
static void stepper_init_instance( stepper_instance *stepper, StepperMotor id )
{
    stepper->position               = 0;
    stepper->fault                  = 0;
    stepper->home_sensor_make_hook  = NULL;
    stepper->home_sensor_break_hook = NULL;
    stepper->alt_sensor_make_hook   = NULL;
    stepper->alt_sensor_break_hook  = NULL;

    switch( id )
    {
    case stepperU:
        stepper->hw_address                 = lid_hw_address;
        stepper->hw_enable_mask             = lid_hw_enable_mask;
        stepper->max_steps                  = max_lid_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = lid_home_position;
        stepper->home_pin_location          = lid_home_pin_location;
        stepper->disable_after_move_at_home = true;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = true;
        stepper->isDBL                      = false;
        break;
    case stepperF:
        stepper->hw_address                 = door_hw_address;
        stepper->hw_enable_mask             = door_hw_enable_mask;
        stepper->max_steps                  = max_door_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = door_home_position;
        stepper->home_pin_location          = door_home_pin_location;
        stepper->disable_after_move_at_home = false;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = true;
        stepper->isDBL                      = false;
        break;
    case stepperR:
        stepper->hw_address                 = filter_hw_address;
        stepper->hw_enable_mask             = filter_hw_enable_mask;
        stepper->max_steps                  = max_filter_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = filter_home_position;
        stepper->home_pin_location          = filter_home_pin_location;
        stepper->disable_after_move_at_home = false;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = false;
        stepper->isDBL                      = false;
        break;
    case stepperD:
        stepper->hw_address                 = lid_hw_address;
        stepper->hw_enable_mask             = lid_hw_enable_mask;
        stepper->max_steps                  = max_lid_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = lid_home_position;
        stepper->home_pin_location          = lid_home_pin_location;
        stepper->disable_after_move_at_home = true;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = true;
        stepper->isDBL                      = true;
        break;
    case stepperB:
        stepper->hw_address                 = door_hw_address;
        stepper->hw_enable_mask             = door_hw_enable_mask;
        stepper->max_steps                  = max_door_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = door_home_position;
        stepper->home_pin_location          = door_home_pin_location;
        stepper->disable_after_move_at_home = false;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = true;
        stepper->isDBL                      = true;
        break;
    case stepperL:
        stepper->hw_address                 = filter_hw_address;
        stepper->hw_enable_mask             = filter_hw_enable_mask;
        stepper->max_steps                  = max_filter_steps;
        stepper->slow_freq                  = defaultSlowFrequency;
        stepper->fast_freq                  = defaultFastFrequency;
        stepper->ramp_steps                 = defaultRampSteps;
        stepper->home_position              = filter_home_position;
        stepper->home_pin_location          = filter_home_pin_location;
        stepper->disable_after_move_at_home = false;
        stepper->disable_after_move_at_alt  = false;
        stepper->disable_after_move_between = true;
        stepper->adjust_home_position       = false;
        stepper->isDBL                      = true;
        break;
    default:
        ;
    }

    stepper_clear_enable_hw_UFR( stepper->hw_enable_mask );
    stepper_clear_enable_hw_DBL( stepper->hw_enable_mask );
}



//------------------------------------------------------------------------------
static void stepper_disable( stepper_instance* moving_stepper )
{
    if(!moving_stepper)
    {
        return;
    }
    if( moving_stepper->position == moving_stepper->home_position )
    {
        // Home
        if( moving_stepper->disable_after_move_at_home )
        {
            if(moving_stepper)
            {
                stepper_clear_enable_hw_UFR( moving_stepper->hw_enable_mask );
                stepper_clear_enable_hw_DBL( moving_stepper->hw_enable_mask );
            }
        }
    }
    else if( moving_stepper->position == stepper_data.target_position )
    {
        // Alt
        if( moving_stepper->disable_after_move_at_alt )
        {
            stepper_clear_enable_hw_UFR( moving_stepper->hw_enable_mask );
            stepper_clear_enable_hw_DBL( moving_stepper->hw_enable_mask );
        }
    }
    else
    {
        // In between
        if( moving_stepper->disable_after_move_between )
        {
            stepper_clear_enable_hw_UFR( moving_stepper->hw_enable_mask );
            stepper_clear_enable_hw_DBL( moving_stepper->hw_enable_mask );
        }
    }
}

static bool isDBLMotor(StepperMotor stepper)
{
    return stepper == stepperD || stepper == stepperB || stepper == stepperL;
}




//EOF
