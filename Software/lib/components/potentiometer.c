#include "potentiometer.h"
#include <peripherals/adc/adc.h>
#include <peripherals/irq/irq.h>
#include <peripherals/aic/aic.h>
#include <boards/mariokartv1/board.h>
#include <char_display.h>

unsigned pot_current_value = 0;
#define BUFFER_SIZE (10)
unsigned pot_value[BUFFER_SIZE];
int buf_pos = 0;

#define BOARD_ADC_FREQ 1000000

//#define POT_CHANNEL ADC_CHANNEL_7 //trace was broken off board. use a different ADC
#define POT_CHANNEL ADC_CHANNEL_7

#define ADC_NEW_VALUE_WAITING_BIT (16)

/**
 * Sets up pins and interrupts for pot, and starts a conversion.
 */
void pot_init(void) {

    //initilize ADC in 10 bit mode at 1MHz frequency
    ADC_Initialize(AT91C_BASE_ADC,
            AT91C_ID_ADC,
            AT91C_ADC_TRGEN_DIS,
            0,
            AT91C_ADC_SLEEP_NORMAL_MODE,
            AT91C_ADC_LOWRES_10_BIT,
            BOARD_MCK,
            BOARD_ADC_FREQ,
            10,
            1200);


    ADC_EnableChannel(AT91C_BASE_ADC, POT_CHANNEL); //enable the potentiometer channel

    AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START; //start ADC an conversion for potentiometer
}

/**
* if a pot adc has been completed then this function saves the ADC value in pot_current_value
* and starts another ADC conversion.
*/
void pot_do_adc(void) {
    //if there is a new adc'd value waiting
	if (AT91C_BASE_ADC->ADC_SR & (1<<ADC_NEW_VALUE_WAITING_BIT) ) {
        //save it in a ring buffer
        buf_pos++;
        buf_pos %= BUFFER_SIZE;
		pot_value[buf_pos] = AT91C_BASE_ADC->ADC_LCDR; //save it

		AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START; //start another ADC
	}
}

/**
* get the current averaged value of the potentiometer
* this is only accurate if do_pot_adc is called often in sucession
*/
unsigned pot_get_value(void) {
    double total;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        total += pot_value[i];
    }
    return (int)(total/BUFFER_SIZE);
}
