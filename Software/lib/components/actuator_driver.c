#include "actuator_driver.h"
#include <boards/mariokartv1/board.h>
#include <peripherals/pio/pio.h>
#include <peripherals/pio/pio_it.h>
#include <peripherals/pwmc/pwmc.h>
#include <peripherals/aic/aic.h>

#include <stdlib.h>
#include <stdio.h>

#include <board.h>
#include <at91sam7xc256/AT91SAM7XC256.h>




//defining actuator pins (note for linar actuator clockwise == extending
//  anticlockwise == retracting)
#define ACT_DRIVER_CHANNEL_CLOCKWISE   0
#define ACT_DRIVER_CHANNEL_ANTICLOCKWISE   1
#define ACT_DRIVER_CHANNEL_ALTERNATE 2


//the motor drivers will not behave well above frequencies of around 20khz
//for frequencies above 10khz the fast slew rate pin needs to be enabled on the motor dirvers
//#define FULL_DUTY_PERIOD (0x01C0) //gives approx 100khz
#define FULL_DUTY_PERIOD (0x2000) //5.85khz - nice and slow.
#define MAX_DUTY_PERIOD (FULL_DUTY_PERIOD - 0xF) /*aparently the motor driver shouldn't be driven at absolute full duty cycle*/
#define MIN_DUTY_PERIOD (0x00)


#define MAX_DUTY_PERCENT (100)

//Specify pin locations
const Pin pwm0_pin = ACT_DRIVER_PWM_CLOCKWISE;
const Pin pwm1_pin = ACT_DRIVER_PWM_ANTICLOCKWISE;
const Pin pwm2_pin = ACT_DRIVER_ALTERNATE_PWM;



/*internal function to set the period (in cycles) of the pwm*/
void set_pwm_duty_period(int pwm_channel, unsigned short period) {
    period = (period <= MAX_DUTY_PERIOD) ? period : MAX_DUTY_PERIOD;
    period = (period >= MIN_DUTY_PERIOD) ? period : MIN_DUTY_PERIOD;
    AT91C_BASE_PWMC->PWMC_CH[pwm_channel].PWMC_CDTYR = period;
}

/*internal function to set the the duty cycle of the pwm to a certain percent for a given channel*/
void set_pwm_duty_percent(int pwm_channel, unsigned short percent) {
    set_pwm_duty_period(pwm_channel, (percent*FULL_DUTY_PERIOD)/100);
}

enum pwm_channel_option {
    USE_ALTERNATE = 0,
    USE_NORMAL = 1
};
enum pwm_channel_option use_normal_pwm = USE_NORMAL;

/**
workaround for non-functioning clockwise pwm channel - use the accelerator PWM channel instead.
MUST be called before act_drive_init
*/
void act_use_alternate_pwm_channel(void) {
    use_normal_pwm = USE_ALTERNATE;
}

//Sets up PWM output
//use_accelerator pwm should generally not be turned on un
void act_driver_init() {

    //sets up driver pins

    if (use_normal_pwm) {
        PIO_Configure(&pwm0_pin, 1);
    } else {
        PIO_Configure(&pwm2_pin, 1);
    }
    PIO_Configure(&pwm1_pin, 1);


    // Enable PWMC peripheral clock
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PWMC; // Power up PWM Controller. 


    if (use_normal_pwm) {
        AT91C_BASE_PWMC->PWMC_DIS = AT91C_PWMC_CHID0; //disable channel 0 - ACT_DRIVER_CHANNEL_CLOCKWISE
    } else {
        AT91C_BASE_PWMC->PWMC_DIS = AT91C_PWMC_CHID2; // disable channel 2 - ACT_DRIVER_ALTERNATE_PWM
    }
    
    
    AT91C_BASE_PWMC->PWMC_DIS = AT91C_PWMC_CHID1; //disable channel 1 - ACT_DRIVER_CHANNEL_ANTICLOCKWISE


    AT91C_BASE_PWMC->PWMC_MR = 0; // Clear mode register.


    if (use_normal_pwm) {
        AT91C_BASE_PWMC->PWMC_CH[0].PWMC_CMR = 0x00 | AT91C_PWMC_CPOL; // use board frequency and polarity 0.
        AT91C_BASE_PWMC->PWMC_CH[0].PWMC_CPRDR = FULL_DUTY_PERIOD; //set the period
        set_pwm_duty_percent(/*channel*/ACT_DRIVER_CHANNEL_CLOCKWISE,0);
    } else {
        AT91C_BASE_PWMC->PWMC_CH[2].PWMC_CMR = 0x00 | AT91C_PWMC_CPOL; // MCK and polarity 0.
        AT91C_BASE_PWMC->PWMC_CH[2].PWMC_CPRDR = FULL_DUTY_PERIOD; //set the period
        set_pwm_duty_percent(/*channel*/ACT_DRIVER_CHANNEL_ALTERNATE,0);
    }

    AT91C_BASE_PWMC->PWMC_CH[1].PWMC_CMR = 0x00 | AT91C_PWMC_CPOL; // MCK and polarity 0.
    AT91C_BASE_PWMC->PWMC_CH[1].PWMC_CPRDR = FULL_DUTY_PERIOD; //set the period
    set_pwm_duty_percent(/*channel*/ACT_DRIVER_CHANNEL_ANTICLOCKWISE,0);


    if (use_normal_pwm) {
        AT91C_BASE_PWMC->PWMC_ENA = AT91C_PWMC_CHID0; // Enable channel 0.
    } else {
        AT91C_BASE_PWMC->PWMC_ENA = AT91C_PWMC_CHID2; // Enable channel 2
    }

    AT91C_BASE_PWMC->PWMC_ENA = AT91C_PWMC_CHID1; // Enable channel 1
}


/*
drive the actuator at a percentage duty cycle; use negative numbers for reverse direction
The steering actuator overcurrents power supplies at 50% duty cycle, and the brake actuator
claimed that it should not be driven over 25% duty cycle in its spec sheet
*/
void act_driver_drive(int percent) {
    if (percent > MAX_DUTY_PERCENT) {
        percent = MAX_DUTY_PERCENT;
    }
    if (percent < -MAX_DUTY_PERCENT) {
        percent = -MAX_DUTY_PERCENT;
    }

    if (percent > 0) {

        if (use_normal_pwm) {
            set_pwm_duty_percent(ACT_DRIVER_CHANNEL_CLOCKWISE,percent);
        } else {
            set_pwm_duty_percent(ACT_DRIVER_CHANNEL_ALTERNATE,percent);
        }

        set_pwm_duty_percent(ACT_DRIVER_CHANNEL_ANTICLOCKWISE,0);
    } else {
        if (use_normal_pwm) {
            set_pwm_duty_percent(ACT_DRIVER_CHANNEL_CLOCKWISE,0);
        } else {
            set_pwm_duty_percent(ACT_DRIVER_CHANNEL_ALTERNATE,0);
        }

        set_pwm_duty_percent(ACT_DRIVER_CHANNEL_ANTICLOCKWISE,-percent);

    }
    /*
    Mat 21/07/2012 - need to somehow make pwm a hard 0 when percent is set to 0.
    might be best to disable, and renable once it's non-zero
    or could alternately try pulling both channels high instead of pulling them both low
    */
}
