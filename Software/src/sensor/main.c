/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2011, University of Canterbury

this board is mainly provided as an extension board for any sensors which might be installed on the kart.
currently it has an inductive proximity sensor used to calculate the speed of the kart.  this speed can be sent over CANBUS
to a requesting board.  Currently the comms board requests the speed value periodically and forwards it onto the USB connection
when requested and the motor board also requests the speed value periodically when performing speed control
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <components/char_display.h>
#include <components/debug.h>
#include <components/switches.h>
#include <protocol/protocol.h>
#include <tc/tc.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <peripherals/tc/tc.h>
#include <aic/aic.h>
#include <speed_sensor.h>

//------------------------------------------------------------------------------
//         Local defines
//------------------------------------------------------------------------------
#define SOFTWARE_NAME "Sensor"

//the timer resolution in ms
#define TIMER_RES 1
#define SPEED_MULTIPLIER (1000)

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

//send the value of the variable var to a board.
//currently only sends speed
void send_data(address_t to, variable_t var) {
    int speed = SPEED_MULTIPLIER*speed_output;

    switch (var) {
        case VAR_SPEED:
            TRACE_INFO("Current kart speed = %i\n\r", speed);

            message_t msg = {
                .from    = ADDR_SENSOR,
                .to      = to,
                .command = CMD_REPLY,
                .data    = {VAR_SPEED, 0, 0, 0, 0}
            };
            int_to_data(speed,msg.data);

            proto_write(msg);

            char_display_number(speed);
            break;

        default:
            //unknown variable
            proto_state_error();
            break;
    }

}

//------------------------------------------------------------------------------
//         Local Functions
//------------------------------------------------------------------------------

//updates time since speed sensor last triggered
void timer_callback(void) {
    speed_update_time(TIMER_RES);
}

//------------------------------------------------------------------------------
//         Main Function
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    variable_t var = 0x0;

    message_t msg = {
        .from    = 0x0,
        .to      = 0x0,
        .command = CMD_NONE,
        .data    = {0x0}
    };

    debug_init(SOFTWARE_NAME);

    //enables interrupts (note resets all configured interrupts)
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);

    //set up speed sensor timing callback
    TC_PeriodicCallback(1000, timer_callback);

    //Main initialisations
    char_display_init();
    switches_init();
    speed_init();

    proto_init(ADDR_SENSOR);

    while (1) {
        char_display_tick();
        switch (proto_state()) {
            case STARTUP:
                break;
            case CALIBRATING:
                proto_calibration_complete();
                break;
            case RUNNING: 
                msg = proto_read();
                switch(msg.command) {
                    case CMD_GET:
                        //a variable was requested, send it
                        var = msg.data[0];
                        send_data(msg.from, var);
                        proto_refresh();
                        break;
                    case CMD_KEEP_ALIVE:
                        proto_refresh();
                    case CMD_NONE:
                        break;
                    default:
                        TRACE_WARNING("Invalid command %i received in running state", msg.command);
                        proto_state_error();
                        break; //Go to error state  
                }

                break;
            default: // ERROR
                break;
        }
    }

    return 0;
}

