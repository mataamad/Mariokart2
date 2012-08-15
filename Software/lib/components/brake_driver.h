#ifndef BRAKE_DRIVER_H
#define BRAKE_DRIVER_H

/**
This file is used to define low level commands to control the brake actuator and get its position.

It uses actuator_driver and potentiometer internally, so these should not be initialized separately
*/


//maximum distance for Actuator in ADC units
#define BRAKE_ACTUATOR_MAXIMUM (1024)
//the maximum acceptable error in position in ADC units. comes to about 0.5mm
#define BRAKE_MAX_POSITION_ERROR (5)
//the position at which the brake is fully engaged
#define BRAKE_FULLY_ON_POSITION (950)

/**
This function intilizes the brake driver and its dependent peripherials
(pot and actuator_driver)
*/
void brake_init(void);

/**
This function does a step for the brake - most importantly it does an ADC for the potentiometer value
so must be called every tick or the value will not be accurate
*/
void brake_do_step(void);


/**
get the current brake actuator position in ADC units.
*/
int brake_get_position(void);

/**
get the desired brake actuator position in ADC units.
*/
int brake_get_desired_position(void);

/**
set the desired brake actuator position in ADC units.
*/
void brake_set_desired_position(int position);

/**
this returns the absolute magnitude of the position error between the current position
and a desired position in ADC units
*/
unsigned brake_get_error_magnitude();

#endif
