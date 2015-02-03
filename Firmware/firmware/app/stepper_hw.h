//-----------------------------------------------------------------------------
//!\file
//!
//!\brief Interface to the Hardware-Dependent Stepper Motors Module
//!
//! This file contains the interface to the hardware-dependent portion of
//! the stepper motors module.  The functions declared here provide the
//! interface to the hardware that the hardware-independent portion of the
//! stepper motors module uses.
//!
//! Copyright (c) 2009 Idaho Technology Inc.
//-----------------------------------------------------------------------------

#ifndef stepper_hw_h
#define stepper_hw_h


#include <types.h>


void InitStepperHW_UFR(void(*step_callback)(void));
void InitStepperHW_DBL(void(*step_callback)(void));
void stepper_set_address_hw(uint32 select);
void stepper_set_enable_hw(uint32 enable);
void stepper_clear_enable_hw(uint32 enable);
void stepper_start_step_hw(uint32 frequency);
bool stepper_get_home_sensor_hw();
bool stepper_get_alt_sensor_hw();
void stepper_clear_direction_hw();
void stepper_set_direction_hw();

void disableMotors();




#endif //...#ifndef stepper_hw_h
