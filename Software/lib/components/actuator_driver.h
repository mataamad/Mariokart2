#ifndef ACTUATOR_DRIVER_H
#define	ACTUATOR_DRIVER_H


/**
workaround for non-functioning clockwise pwm channel - use the accelerator PWM channel instead.
MUST be called before act_drive_init
*/
void act_use_alternate_pwm_channel(void);

/**
 * Sets up PWM output
 */
void act_driver_init(void);


/**
 * Takes in a number between 100 and -100 and uses it
 * to set PWM for driving the actuator where
 * 100 = full speed clockwise
 * 50 = half speed clockwise
 * 0 = stopped
 * -100 = full speed anticlockwise etc
 *
 * The steering actuator overcurrents power supplies at 50% duty cycle, and the brake actuator
 * claimed that it should not be driven over 25% duty cycle in its spec sheet
 *
 * @param percent     The desired PWM as specified above
 */
void act_driver_drive(int percent);

#endif	/* ACTUATOR_DRIVER_H */

