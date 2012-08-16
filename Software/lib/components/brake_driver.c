#include "brake_driver.h"

#include <actuator_driver.h>
#include <potentiometer.h>
#include <utility/pid.h>


#define BRAKE_PROPORTIONAL_GAIN (1.0)
#define MINIMUM_DRIVE_PERCENT (25)
#define MAX_DRIVE_PERCENT (75)

//initially drive to the 'full on' brake position
static int desired_brake_position = BRAKE_FULLY_ON_POSITION;


/**
get the desired brake actuator position in ADC units.
*/
int brake_get_desired_position(void) {
    return desired_brake_position;
}

/**
set the desired brake actuator position in ADC units.
*/
void brake_set_desired_position(int position) {
    if (position >= 0 && position <= BRAKE_ACTUATOR_MAXIMUM) {
        desired_brake_position = position;
    }
}

/**
This function intilizes the brake driver and its dependent peripherials
(pot and actuator_driver)
*/
void brake_init(void) {
    pot_init(); 
    act_driver_init();
}


/**
This function does a step for the brake.
It does an ADC for the potentiometer value and uses P control to driver the actuator towards its desired position
so must be called every tick or the value will not be accurate and the P control will not work.
*/
void brake_do_step(void) {
    //attempt to do a pot ADC conversion
    pot_do_adc();

    //drive the brake to a position using proportional control
    act_driver_drive(do_drive_calculation(desired_brake_position));
}

/**
calculates the percent duty cycle to drive the brake motor to using P control
*/
int do_drive_calculation() {
    int current_position = pot_get_value();
    int drive_percent = pid_calculate_P_gain(desired_brake_position, current_position, BRAKE_PROPORTIONAL_GAIN);

    //limit the duty cycle
    if (drive_percent < -MAX_DRIVE_PERCENT) {
        drive_percent = -MAX_DRIVE_PERCENT;
    }
    if (drive_percent > MAX_DRIVE_PERCENT) {
        drive_percent = MAX_DRIVE_PERCENT;
    }

    if (drive_percent < 5 && drive_percent > 5) {
        drive_percent = 0;
    }
    
    //set the duty cycle to the minimum acceptable one for values < 25% - this will cause overshoot but it cannot be helped
    if (drive_percent > 0 && drive_percent < MINIMUM_DRIVE_PERCENT) {
        drive_percent = MINIMUM_DRIVE_PERCENT;
    }
    if (drive_percent < 0 && drive_percent > -MINIMUM_DRIVE_PERCENT) {
         drive_percent = -MINIMUM_DRIVE_PERCENT;
    }


    if (brake_get_error_magnitude() < BRAKE_MAX_POSITION_ERROR) {
        drive_percent = 0;
    }

    return drive_percent;
}

/**
get the current brake position in mm
*/
int brake_get_position(void) {
    return pot_get_value();
}

/**
this returns the absolute magnitude of the position error between the current position
and a desired position in ADC units
*/
unsigned brake_get_error_magnitude() {
    int error = pot_get_value() - desired_brake_position;
    if (error < 0) {
        error = -error;
    }
    return error;
}



