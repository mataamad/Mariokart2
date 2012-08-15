/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012, University of Canterbury

This module is the main file for the brake board.
It uses brake_driver.h to perform proportional control on the brake actuator.

This module waits for set position commands from other modules, and (if the value is valid)
sets the position of the brake actoator to follow the requested position.

This module also facilitates getting the current position of the brake (in ADC units - 0 to 1024)
via a CANBUS request.

During calibration the brake is moved to the 'full on' position.

 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <char_display.h>
#include <tc/tc.h>
#include <debug.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <protocol/protocol_master.h>
#include <potentiometer.h>
#include <brake_driver.h>
#include <switches.h>

//------------------------------------------------------------------------------
//         Local defines
//------------------------------------------------------------------------------
#define SOFTWARE_NAME ("MatsBrake")


//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------


/**
acknowledge that a command was received.
*/
void send_ack(message_t orig_msg) {
    message_t msg = {
        .from     = ADDR_BRAKE,
        .to       = orig_msg.from,
        .command  = CMD_ACK_SET,
        .data[0]  = orig_msg.data[0],
    };
    proto_write(msg);
}

/**
send the current brake position in ADC units to the requesting address
*/
void send_current_brake_position(address_t send_to) {
    message_t msg = {
        .from     = ADDR_BRAKE,
        .to       = send_to,
        .command  = CMD_REPLY,
        .data  = { VAR_BRK_POS, 0, 0, 0, 0}
    };
    int brake_position = brake_get_position();
    int_to_data(brake_position, msg.data);
    proto_write(msg);
}


/**
Do a sigle tick of the canbus protocol handling - startup, calibrating, and listening to and responding to messages
*/
void do_protocol_tick(void) {
    message_t msg;
    variable_t variable;
    int position_error;

    switch (proto_state()) {
        case STARTUP:

            break;
        case CALIBRATING:
#ifndef SKIP_CALIBRATION
            position_error = brake_get_error_magnitude();
            if (position_error < BRAKE_MAX_POSITION_ERROR) {
                proto_calibration_complete();
            }
#else
            proto_calibration_complete();
#endif
            break;
        case RUNNING:
            msg = proto_read();
            variable = msg.data[0];
            switch(msg.command) {

                case CMD_SET:
                    //set brake position
                    if (variable == VAR_BRK_POS) {
                        brake_set_desired_position(data_to_int(msg.data));
                    } else {
                        proto_state_error();
                    }
                    send_ack(msg);
                    proto_refresh();
                    break;

                case CMD_GET:
                    //get brake position
                    if (variable == VAR_BRK_POS ) {
                        send_current_brake_position(msg.from);
                    } else {
                        proto_state_error();
                    }
                    proto_refresh();

                    break;
                case CMD_KEEP_ALIVE: //refresh the protocol
                    proto_refresh();
                    break;
                case CMD_NONE: //this is received when no message is waiting
                    break;
                default:
                    TRACE_WARNING("Invalid command %i received in running state\n\r", msg.command);
                    proto_state_error();
                    break;
            }
            break;
        default: // ERROR
            //apply the brake
            brake_set_desired_position(BRAKE_FULLY_ON_POSITION);
            break;
    }
}


/**
callback to update the display periodically
*/
void display_callback() {
    char_display_tick();
}

/**
debug function to show the brake position (desired or current) on the 7 segment displays
*/
void display_info() {
    //display desired brake position
    if (brake_get_desired_position()/10 >= 0 && brake_get_desired_position()/10 <= 99) {
        char_display_number(brake_get_desired_position()/10);
    } else {
        char_display_number(99);
    }

    /*//display the current brake position truncating to max 99.
    if (brake_get_position()/10 < 99) {
        char_display_number(brake_get_position()/10);
    } else {
        char_display_number(99);
    }*/
}


//------------------------------------------------------------------------------
//         Main Function
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    debug_init(SOFTWARE_NAME);

    //enables interrupts (note resets all configured interrupts)
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);

    //Main initialisations
    char_display_init();
    switches_init();

    brake_init();

    // Starts a 100Hz display update timer
    TC_PeriodicCallback(100, display_callback);

    proto_init(ADDR_BRAKE);

    while (1) {
        //do an ADC step - needs to be called each tick.
	    pot_do_adc();

        brake_do_step();


        //do a tick involving the protocol functions
        do_protocol_tick();

        //display_info();

    }
    return 0;
}

