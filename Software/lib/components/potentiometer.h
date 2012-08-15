#ifndef POTENTIOMETER_H
#define	POTENTIOMETER_H

/**
This module defines the control for the potentiometer which supplies the brake position.

Note that pot_get_value returns a 5-value moving average to help reduce sensor noise
so at least 5 adc calls must be made before pot_get_value returns valid values

*/


/**
 * Sets up pins and interrupts for pot, and starts a conversion.
 */
void pot_init(void);
/**
* if a pot adc has been completed then this function saves the ADC value in pot_current_value
* and starts another ADC conversion.
*/
void pot_do_adc(void);

/**
* get the current averaged value of the potentiometer
* this is only accurate if do_pot_adc is called often in sucession
*/
unsigned pot_get_value(void);

#endif	/* POTENTIOMETER_H */
