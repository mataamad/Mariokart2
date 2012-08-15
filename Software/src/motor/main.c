/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2011, University of Canterbury

This board controls the acceleration of the kart.
currently it does this by interfacing with the student board via SPI
(the student board has to be reflashed to use the SPI code)
It allows two options of control:
open loop acceleration control
closed loop speed control
the acceleration control merely sends out any incoming USB SetAcceleration signals via SPI.
The closed loop speed control takes a target speed as input via USB SetSpeed() and uses proportional control
to set the kart speed to this value

The acceleration value is measured as a % of maximum acceleration between 0 and 256.
The maximum acceleration value that can be used is 220.
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <components/char_display.h>
#include <components/debug.h>
#include <components/switches.h>
#include <board.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <protocol/protocol.h>
#include <spi/spi.h>
#include <tc/tc.h>
#include <utility/trace.h>
#include <utility/pid.h>

//------------------------------------------------------------------------------
//         Local defines
//------------------------------------------------------------------------------
#define SOFTWARE_NAME "Motor"

#define MAX_ACCELERATION (255)
#define MAX_ALLOWBLE_ACCELERATION (220)

//Sets the chip select for the student board SPI
#define STUDENT_CS 0x0

//The Student board SPI baud rate
//#define STUDENT_BAUD 128000
#define STUDENT_BAUD 8000

//proportional gain to use when doing speed control
#define SPEED_CONTROL_PROPORTIONAL_GAIN (1.0)

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
static const Pin student_spi_pins[] = {PINS_SPI1,PIN_SPI1_NPCS0,PIN_SPI1_NPCS1,PIN_SPI1_NPCS2};
AT91PS_SPI student_spi = AT91C_BASE_SPI1;



//type of control to use
typedef enum {
    SPEED_CONTROL,
    ACCELERATION_CONTROL,
} control_type_t;

static int kart_speed = 0; //current kart speed in 1000*m/s
static int target_speed = 0;
static int target_acceleration = 0;
static control_type_t control_type = SPEED_CONTROL; //type of control to use - acceleration or speed control



//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//send a request to the sensor board for its speed value
void request_speed_value() {
    static const message_t msg = {
        .from = ADDR_MOTOR,
        .to   = ADDR_SENSOR,
        .command = CMD_GET,
        .data = { VAR_SPEED, 0, 0, 0, 0 }
    };
    proto_write(msg);
}

void set_motor(unsigned int setpoint) {
    // Make sure the value is valid else error
    if (setpoint > MAX_ALLOWBLE_ACCELERATION) {
        TRACE_WARNING("The accelerator value (%d) sent to the student board is invalid!\n\r", setpoint);
        SPI_Write(student_spi, STUDENT_CS, 0x0000);
        SPI_Write_better(student_spi, STUDENT_CS, 0);
        proto_state_error();
    }
    else {
        TRACE_DEBUG("Setting the motor board accelerator value to: %d\n\r", setpoint);
        SPI_Write_better(student_spi, STUDENT_CS, setpoint);
    }

}

void init_student_spi() {
    TRACE_DEBUG("Configuring SPI for student board\n\r");
    PIO_Configure(student_spi_pins, PIO_LISTSIZE(student_spi_pins));
    //set 12 bits per transmit for max value of `TOP_ACCELERATOR'
    /*SPI_Configure(
            student_spi,
            AT91C_ID_SPI1,
            AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED
            );*/

    //student_spi->SCBR = 255; //hmmm HOW?


    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI1;
    student_spi->SPI_CR = AT91C_SPI_SPIDIS;
    // Execute a software reset of the SPI twice
    student_spi->SPI_CR = AT91C_SPI_SWRST;
    student_spi->SPI_CR = AT91C_SPI_SWRST;
    student_spi->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED;


    /*SPI_ConfigureNPCS(
            student_spi,
            STUDENT_CS,
            AT91C_SPI_NCPHA | AT91C_SPI_BITS_12 | SPI_SCBR(STUDENT_BAUD, BOARD_MCK)
            );*/

    //student_spi->SPI_CSR[STUDENT_CS] = AT91C_SPI_NCPHA | AT91C_SPI_BITS_12 | SPI_SCBR(STUDENT_BAUD, BOARD_MCK);
    //SCBR
    student_spi->SPI_CSR[STUDENT_CS] = AT91C_SPI_NCPHA | AT91C_SPI_BITS_8 | (0x0000FF00);

    /*SPI_Enable(student_spi);*/
    student_spi->SPI_CR = AT91C_SPI_SPIEN;
}

void timer_callback(void) {
    char_display_tick();
}

void send_ack(message_t orig_msg) {
    message_t msg = {
        .from     = ADDR_MOTOR,
        .to       = orig_msg.from,
        .command  = CMD_ACK_SET,
        .data[0]  = orig_msg.data[0],
    };
    proto_write(msg);
}

/*
uses P control to accelerate the go-kart towards the target speed
*/
void do_speed_control_step(void) {
    int drive_percent = pid_calculate_P_gain(target_speed, kart_speed, SPEED_CONTROL_PROPORTIONAL_GAIN);

    int acceleration = (MAX_ALLOWBLE_ACCELERATION*drive_percent)/100;
  
    if (acceleration < 0) {
        acceleration = 0;
    }
    if (acceleration > MAX_ALLOWBLE_ACCELERATION) {
        acceleration = MAX_ALLOWBLE_ACCELERATION;
    }

    set_motor(acceleration);
}

/*void wait_time(int us) {
    int tgt = us*5000;
    for (volatile int i = 0; i < tgt; i++) {
      
    }

}*/

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
    init_student_spi();

    //Starts a timer running at 100Hz to maintain the display
    TC_PeriodicCallback(100, timer_callback);

    message_t msg;

    proto_init(ADDR_MOTOR);
    int i;

    while (1) {

        switch (proto_state()) {
            case STARTUP:
                break;
            case CALIBRATING:
                proto_calibration_complete();
                break;
            case RUNNING:

                //periodically get the speed of the kart from the sensor board
                if (i % 9999 == 0) {
                    request_speed_value();
                }
                i++;
                i %= 10000;

                msg = proto_read();
                switch(msg.command) {
                    case CMD_SET:
                        switch (msg.data[0]) {
                            case VAR_SPEED:
                                control_type = SPEED_CONTROL;
                                target_speed = data_to_int(msg.data);
                                target_acceleration = 0;

                                break;
                            case VAR_ACCELERATION:
                                control_type = ACCELERATION_CONTROL;
                                target_acceleration = data_to_int(msg.data);
                                if (target_acceleration < 0) {
                                    target_acceleration = 0;
                                }
                                if (target_acceleration > MAX_ALLOWBLE_ACCELERATION) {
                                    target_acceleration = MAX_ALLOWBLE_ACCELERATION;
                                }
                                set_motor(target_acceleration);
                                target_speed = 0;
                                break;
                            default:
                                proto_state_error();
                                break;
                        }
                        send_ack(msg);
                        proto_refresh();
                        break;

                    /**
                    checks if the sensor board has sent a speed response, and if so saves it
                    */
                    case CMD_REPLY:
                        switch (msg.data[0]) {
                            case VAR_SPEED:
                                kart_speed = data_to_int(msg.data);
                                break;
                            default:
                                proto_state_error();
                                break;
                        }

                    case CMD_KEEP_ALIVE:
                        proto_refresh();
                    case CMD_NONE:
                        break;
                    default:
                        TRACE_WARNING("Invalid command %i received in running state", msg.command);
                        proto_state_error();
                        break; 
                }

                if (control_type == SPEED_CONTROL) {
                    do_speed_control_step();
                }

                break;
            default: // ERROR

                    SPI_Write_better(student_spi, STUDENT_CS, 0);

                    //set_motor(0);

                break;
        }
    }
    return 0;
}

