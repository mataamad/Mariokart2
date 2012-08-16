/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012, University of Canterbury
 *
 * communications board main code.  Handles board startup and calibration, gets data from the other boards
 * and sends it on to the computer via USB.  ensures that the boards respond to periodic polling.
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <components/char_display.h>
#include <components/debug.h>
#include <components/switches.h>
#include <protocol/protocol_master.h>
#include <tc/tc.h>
#include <aic/aic.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <pmc/pmc.h>
#include <utility/trace.h>
#include <pit/pit.h>
#include "usb.c"

//------------------------------------------------------------------------------
//         Local defines
//------------------------------------------------------------------------------
#define SOFTWARE_NAME "Comms"
//#define ALL_CLIENTS ((1<<ADDR_BRAKE) | (1<<ADDR_MOTOR)| (1<<ADDR_STEERING) )
//#define ALL_CLIENTS (1<<ADDR_BRAKE)
//#define ALL_CLIENTS ((1<<ADDR_BRAKE) | (1<<ADDR_STEERING))

#define ALL_CLIENTS ((1<<ADDR_BRAKE) | (1<<ADDR_STEERING) | (1<<ADDR_SENSOR) | (1<<ADDR_MOTOR))
//#define ALL_CLIENTS ((1<<ADDR_MOTOR) | (1<<ADDR_SENSOR))

//define ALL_CLIENTS ((1<<ADDR_BRAKE) | (1<<ADDR_SENSOR) | (1<<ADDR_MOTOR))


#define KEEPALIVE_TIMEOUT_MULTIPLIER (10)
volatile bool timeout = false; //periodically set to true for running events at a constant rate

static int brake_position = 0;
static int steering_angle = 0;
static int kart_speed = 0;

unsigned board_keep_alive_responses = ALL_CLIENTS;


/**
message used to broadcast a command to all boards
*/
message_t broadcast_message = {
    .from     = ADDR_COMMS,
    .to       = ADDR_BROADCAST_RX,
    .command  = CMD_NONE,
    .data_len = 0
};


//------------------------------------------------------------------------------
//         Functions
//------------------------------------------------------------------------------

/**
 * Blocks until the most recent message sent or times out
 * @return true if the message sent, false otherwise
 */
/*bool wait_on_ack(message_t msg) {
    transmission_wait = 0;
    while ( (received_ack.from != msg.to) && (received_ack.data[0] != msg.data[0]) ) {
        if (transmission_wait > 3) {
            BCAN_AbortTransfer(0, msg.to);
            return false;
        }
    }
    return true;
}*/


//send a request to the brake board for its value
void request_brake_value() {
    static const message_t brake_get_msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_BRAKE,
        .command = CMD_GET,
        .data = { VAR_BRK_POS, 0, 0, 0, 0 }
    };

    proto_write(brake_get_msg);
}

//send a request to the steering board for its value
void request_steering_value() {
    static const message_t msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_STEERING,
        .command = CMD_GET,
        .data = { VAR_STEERING_ANGLE, 0, 0, 0, 0 }
    };
    proto_write(msg);
}


//send a request to the sensor board for its speed value
void request_speed_value() {
    static const message_t msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_SENSOR,
        .command = CMD_GET,
        .data = { VAR_SPEED, 0, 0, 0, 0 }
    };
    proto_write(msg);
}

void send_motor_keep_alive() {
    static const message_t msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_MOTOR,
        .command = CMD_KEEP_ALIVE,
        .data = { 0, 0, 0, 0, 0 }
    };
    proto_write(msg);
}

//send a reply to the USB controller containing the value of the variable requested
void usb_send_reply(variable_t variable, int value) {
    message_t msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_COMMS_USB,
        .command = CMD_REPLY,
        .data = { variable, 0, 0, 0, 0 }
    };
    int_to_data(value, msg.data);

    UsbWrite(msg);
}


/**
broadcast a message to USB to indicate that the system has entered an error state
*/
void usb_send_error() {
    static message_t msg = {
        .from = ADDR_COMMS,
        .to   = ADDR_COMMS_USB,
        .command = CMD_ERROR,
        .data = { 0, 0, 0, 0, 0 }
    };
    UsbWrite(msg);
}

/**
* timer which i called at 100hz.
It refreshes the display and updates a timeout flag which is used to periodically run events in the bakcground
*/
void timer_callback(void) {
    static int i = 0;

    // Update the display
    char_display_tick();
    AT91C_BASE_TC1->TC_SR;
    if (i++ > 10) {
        // A flag used to run periodic events in the background
        timeout = true;
        i = 0;
    }
}

/**
This function contains the code to do either a calibrate or a startup step.
It waits for affirmatory responses from all the boards, and if so moves on to the next stage

returns the responses which have returned affirmatory
*/
unsigned check_for_board_responses(unsigned responses, command_t response_command) {
    // Check any responses
    message_t msg = proto_read();
    if (msg.command == response_command) {
            responses |= 1 << msg.from; 
    } else if (msg.command != CMD_NONE && msg.command != CMD_NO) {
            proto_state_error();
    }

    return responses;
} 

/**
perform a startup step. periodically probe for boards until all have responded - and then move on to calibration stage
returns the 'responses' data which records which boards have responded affirmatory
*/
unsigned do_startup_step(unsigned responses) {

    // Probe all boards periodically
    if (timeout) {
        responses = 0;
        broadcast_message.command = CMD_REQ_CALIBRATE;
        proto_write(broadcast_message);
        timeout = false;
    } 

    responses = check_for_board_responses(responses, CMD_ACK_CALIBRATE);

    // Check to see if all boards have acked
    if (responses == ALL_CLIENTS) {
        broadcast_message.command = CMD_CALIBRATE;
        proto_write(broadcast_message);
        proto_state_transition(CALIBRATING);
        responses = 0;
    }

    return responses;
}

/**
similar to the startup step, this function polls the boards, and waits until they are all ready to start running
and transitions to the running state.
returns the 'responses' data which records which boards have responded affirmatory
*/
unsigned do_calibration_step(unsigned responses) {
    // Request a transition periodically
    if (timeout) {
        responses = 0;
        broadcast_message.command = CMD_REQ_RUN;
        proto_write(broadcast_message);
        timeout = false;
    } 

    responses = check_for_board_responses(responses, CMD_ACK_RUN);

    // Check if all clients have acked
    if (responses == ALL_CLIENTS) {
        broadcast_message.command = CMD_RUN;
        proto_write(broadcast_message);
        proto_state_transition(RUNNING);
    }

    return responses;
}



/**
Actions performed each step involving USB.
The USB channel is monitored for messages.  If data has been requested then it is sent back to the computer.
If a 'set' command is received it is forwaded through to the board it was intended for
*/
void do_usb_step(void) {
    message_t usb_msg = UsbRead();

    switch (usb_msg.command) {
        case CMD_NONE:
            break;
        case CMD_GET:
            //on get commands send the USB controller the locally cached variable value
            switch (usb_msg.data[0]) {
                case VAR_BRK_POS:
                    usb_send_reply(VAR_BRK_POS, brake_position);
                    break;
                case VAR_STEERING_ANGLE:
                    usb_send_reply(VAR_STEERING_ANGLE, steering_angle);
                    break;
                case VAR_SPEED:
                    usb_send_reply(VAR_SPEED, kart_speed);
                    break;
                default:
                    proto_state_error();
                    break;
            }

            break; 
        case CMD_SET:
            //for set commands we can simply forward the message on to the intended recipient because
            //the server side USB code has set the .to to the board it wants ot send the command to
            proto_write(usb_msg);
            break;
        default:
            proto_state_error();
            break;
    }
}

/**
Respond to any usb commands with an error command.
Used to signal to USB that the boards are in an error state 
*/
void respond_to_usb_with_errors(void) {
    message_t usb_msg = UsbRead();
    if (usb_msg.command != CMD_NONE) {
        usb_send_error();
    }
}

/**
This function checks the canbus for messages containing replies to requests for information about variables
and saves them in file variables
*/
void check_CANBUS_messages_step() {
    message_t msg;

    // Check any responses
    do {
        msg = proto_read();
        switch(msg.command) {
            case CMD_NONE:
                break;
                
            case CMD_REPLY:
                board_keep_alive_responses |= 1 << msg.from;
                switch (msg.data[0]) {
                    case VAR_BRK_POS:
                        brake_position = data_to_int(msg.data);
                        break;
                    case VAR_STEERING_ANGLE:
                        steering_angle = data_to_int(msg.data);
                        break;
                    case VAR_SPEED:
                        kart_speed = data_to_int(msg.data);
                        break;
                    default:
                        proto_state_error();
                        break;
                }

                break;

            case CMD_ACK_SET:
                board_keep_alive_responses |= 1 << msg.from;
                break;

            case CMD_KEEP_ALIVE:
                board_keep_alive_responses |= 1 << msg.from;
                break;

            // Invalid response
            default:
                TRACE_ERROR("Invalid command %i received in calibrating state\n\r", msg.command);
                proto_state_error();
                break;
        }
    } while (msg.command != CMD_NONE);
}

int main(int argc, char *argv[]) {
    debug_init(SOFTWARE_NAME);

    //enables interrupts (note resets all configured interrupts)
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);

    //start periodic timer for char display update etc
    TC_PeriodicCallback(100, timer_callback);

    //Main initialisations
    char_display_init();
    //switches_init();
    UsbInit();
    proto_init(ADDR_COMMS);


    unsigned responses = 0;
    unsigned i = 0;

    while(1) {    
        switch (proto_state()) {
            case STARTUP:
                responses = do_startup_step(responses);
                break;
            case CALIBRATING:
                responses = do_calibration_step(responses);
                break;
            case RUNNING: // Normal state

                do_usb_step();

                check_CANBUS_messages_step(); //check for new messages

                if (timeout) {
                    //periodically poll the peripherals, and also request more data from them
                    request_brake_value();
                    request_steering_value();
                    request_speed_value();
                    send_motor_keep_alive();
                    
                    proto_refresh();

                    i++;
                    if (i == KEEPALIVE_TIMEOUT_MULTIPLIER) {
                    
                        if (board_keep_alive_responses != ALL_CLIENTS) {
                            proto_state_error();
                        }

                        board_keep_alive_responses = 0;
                    }
                    i %= KEEPALIVE_TIMEOUT_MULTIPLIER;



                }

                break;
            default: // ERROR
                if (timeout) {
                    responses = 0;
                    broadcast_message.command = CMD_ERROR;
                    proto_write(broadcast_message); //broadcast error message to a connected components
                    usb_send_error();
                    timeout = false;
                } 
                respond_to_usb_with_errors(); //send only errors to usb
                break;
        }
    }
}

