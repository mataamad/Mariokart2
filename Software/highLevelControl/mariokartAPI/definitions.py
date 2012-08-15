'''
Mariokart project

Copyright 2012 University of Canterbury

Author: Matthew Wigley

This module contains definitions of commands and addresses for controlling the boards
It MUST be kept identical with definitions.h

Currently all communications via CANBUS and USB in this project use the following message format:
1 byte: from address
1 byte: to address
1 byte: command
1 byte: data length
5 bytes: data (the first byte is the name of the variable, the next 4 bytes make up little endian 32 bit signed integer value)
'''

#addresses
ADDR_ERROR_RX     = '\x00'
ADDR_BROADCAST_RX = '\x01'
ADDR_BROADCAST_TX = '\x02'
ADDR_BRAKE        = '\x03'
ADDR_COMMS        = '\x04'
ADDR_COMMS_USB    = '\x04'
ADDR_STEERING     = '\x05'
ADDR_MOTOR        = '\x06'
ADDR_SENSOR       = '\x07'

#commands
CMD_NONE          = '\x00'
CMD_GET           = '\x01'
CMD_REPLY         = '\x02'
CMD_SET           = '\x03'
CMD_REQ_CALIBRATE = '\x04'
CMD_ACK_CALIBRATE = '\x05'
CMD_REQ_RUN       = '\x06'
CMD_ACK_RUN       = '\x07'
CMD_NO            = '\x08'
CMD_ERROR         = '\x09'
CMD_RUN           = '\x0A'
CMD_CALIBRATE     = '\x0B'
CMD_ACK_SET       = '\x0C'
CMD_KEEP_ALIVE    = '\x0D'

#variable names
VAR_SPEED          = '\x01'
VAR_BRK_POS        = '\x02'
VAR_STEERING_ANGLE = '\x03'
VAR_ACCELERATION   = '\x04'

#data length in a Set command
DATA_LENGTH_SET    = '\x05'
#data length in a get command
DATA_LENGTH_GET    = '\x01'
