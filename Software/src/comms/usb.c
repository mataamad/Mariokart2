/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2011, University of Canterbury
 */

/******************************************************************************
 * Interface used by main to interact with this section of code.              *
 ******************************************************************************/

// Initializes the usb connection.
static inline void UsbInit();

// Initiates a read on the usb and returns the result of the last read.
static message_t UsbRead();


/******************************************************************************
 * Inner workings, this is so inextricably linked to the main comms code that *
 * there is little reason to seperate out into an actual module.              *
 ******************************************************************************/
static volatile message_t usb_received_msg;

#include <board.h>
#include <utility/interrupts.h>
#include <usb/device/cdc-serial/CDCDSerialDriver.h>
#include <usb/device/cdc-serial/CDCDSerialDriverDescriptors.h>

// Size in bytes of the buffer used for reading data from the USB.
#define DATABUFFERSIZE \
    BOARD_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_DATAIN)
#define MESSAGEBUFFERSIZE 10

// Buffer for storing incoming USB data.
static unsigned char usbBuffer[DATABUFFERSIZE];

// Buffer for storing parts of the message until it is complete.
static unsigned char message_buffer[MESSAGEBUFFERSIZE] = {0};

// Current position in the buffer.
static unsigned int message_buffer_position = 0;

#ifdef PIN_USB_VBUS
/**
 * Handles interrupts coming from the PIO controller for the VBus pin
 */
static void ISR_VBus(const Pin *pPin) {
    if (PIO_Get(pPin)) {
        TRACE_INFO("VBUS connected\n\r");
        USBD_Connect();
    } else {
        TRACE_INFO("VBUS disconnected\n\r");
        USBD_Disconnect();
    }
}
#endif

/* If PIN_USB_VBUS defined configures the PIO controller to
 * trigger an interrupt when the level on the VBus pin changes */
static void VBus_Configure() {
#ifdef PIN_USB_VBUS
    static const Pin pinVBus = PIN_USB_VBUS;

    TRACE_INFO("VBus configuration\n\r");

    PIO_Configure(&pinVBus, 1);
    PIO_ConfigureIt(&pinVBus, ISR_VBus);
    PIO_EnableIt(&pinVBus);

    ISR_VBus(&pinVBus);
#else
    USBD_Connect();
#endif
}

static inline void UsbInit() {
    CDCDSerialDriver_Initialize();
    VBus_Configure();
}

// Message format is same as message_t
static message_t parse_usb_message(const unsigned char message[], unsigned int length) {
    //char_display_number(15);

    message_t msg = {
        .from     = ADDR_ERROR_RX,
        .to       = ADDR_ERROR_RX,
        .command  = CMD_NONE,
        .data     = {0},
        .data_len = 0
    };

    //char_display_number(length);

    if (length >= 9) {

        msg.from     = message[0];
        msg.to       = message[1];
        msg.command  = message[2];
        msg.data_len = message[3];
        msg.data[0]  = message[4];
        msg.data[1]  = message[5];
        msg.data[2]  = message[6];
        msg.data[3]  = message[7];
        msg.data[4]  = message[8];
    }

    return msg;
}

static void UsbHandler(const unsigned char data[], unsigned int length) {

    if (length > (MESSAGEBUFFERSIZE - message_buffer_position)) {
        TRACE_WARNING("Too long USB message received.\n\r");
        message_buffer_position = 0;
    } else {
        for (unsigned int i = 0; i < length; i++) {
            message_buffer[message_buffer_position + i] = data[i];
        }
        message_buffer_position += length;
        if (data[length-1] == 0xFF) {
            usb_received_msg = parse_usb_message(message_buffer, message_buffer_position);
            if (usb_received_msg.command == CMD_NONE) {
                TRACE_WARNING("Invalid USB message received.\n\r");
            } else {
                TRACE_DEBUG("USB message received:" "\n\r");
                TRACE_DEBUG("    to:       0x%02X" "\n\r", usb_received_msg.to);
                TRACE_DEBUG("    command:  0x%02X" "\n\r", usb_received_msg.command);
                TRACE_DEBUG("    data_len: 0x%02X" "\n\r", usb_received_msg.data_len);
                TRACE_DEBUG("    data:          " "\n\r");
                TRACE_DEBUG("              0x%02X" "\n\r", usb_received_msg.data[0]);
                TRACE_DEBUG("              0x%02X" "\n\r", usb_received_msg.data[1]);
                TRACE_DEBUG("              0x%02X" "\n\r", usb_received_msg.data[2]);
                TRACE_DEBUG("              0x%02X" "\n\r", usb_received_msg.data[3]);
                TRACE_DEBUG("              0x%02X" "\n\r", usb_received_msg.data[4]);
            }
            message_buffer_position = 0;
        } else {
            TRACE_INFO("Incomplete message\n\r");
        }
    }
}

static void UsbDataReceived(void *unused,
                     unsigned char status,
                     unsigned int received,
                     unsigned int remaining)
{
    // Check that data has been received successfully
    if (status == USBD_STATUS_SUCCESS) {

        TRACE_INFO("UsbDataReceived:\n\r[\n\r");
        for (unsigned int i = 0; i < received-1; i++) {
            TRACE_INFO("\t0x%02X\n\r", usbBuffer[i]);
        }
        TRACE_INFO("\t0x%02X\n\r]\n\r", usbBuffer[received-1]);

        UsbHandler(usbBuffer, received);

        // Check if bytes have been discarded
        if ((received == DATABUFFERSIZE) && (remaining > 0)) {
            TRACE_WARNING(
                      "UsbDataReceived: %u bytes discarded\n\r",
                      remaining);
        }
    }
    else {
        TRACE_WARNING( "UsbDataReceived: Transfer error\n\r");
    }
}

static message_t UsbRead() {
    disable_interrupts();

    message_t temp = usb_received_msg;

    usb_received_msg.from     = ADDR_ERROR_RX;
    usb_received_msg.to       = ADDR_ERROR_RX;
    usb_received_msg.command  = CMD_NONE;
    usb_received_msg.data[0]  = 0;
    usb_received_msg.data[1]  = 0;
    usb_received_msg.data[2]  = 0;
    usb_received_msg.data[3]  = 0;
    usb_received_msg.data[4]  = 0;
    usb_received_msg.data_len = 0;

    enable_interrupts();

    CDCDSerialDriver_Read(usbBuffer, DATABUFFERSIZE, UsbDataReceived, 0);

    return temp;
}

void UsbWrite(message_t message) {
    CDCDSerialDriver_Write(&message,sizeof(message),0,0);
}
