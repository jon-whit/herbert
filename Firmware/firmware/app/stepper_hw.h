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
//-----------------------------------------------------------------------------

#ifndef stepper_hw_h
#define stepper_hw_h

#include <types.h>

void init_stepper_hw_UFR(void(*step_callback)(void));
void stepper_set_address_hw_UFR(uint32 select);
void stepper_set_enable_hw_UFR(uint32 enable);
void stepper_clear_enable_hw_UFR(uint32 enable);
void stepper_start_step_hw_UFR(uint32 frequency);
bool stepper_get_home_sensor_hw_UFR();
bool stepper_get_alt_sensor_hw_UFR();
void stepper_clear_direction_hw_UFR();
void stepper_set_direction_hw_UFR();

void init_stepper_hw_DBL(void(*step_callback)(void));
void stepper_set_address_hw_DBL(uint32 select);
void stepper_set_enable_hw_DBL(uint32 enable);
void stepper_clear_enable_hw_DBL(uint32 enable);
void stepper_start_step_hw_DBL(uint32 frequency);
bool stepper_get_home_sensor_hw_DBL();
bool stepper_get_alt_sensor_hw_DBL();
void stepper_clear_direction_hw_DBL();
void stepper_set_direction_hw_DBL();

void disableMotors();

#endif //...#ifndef stepper_hw_h
