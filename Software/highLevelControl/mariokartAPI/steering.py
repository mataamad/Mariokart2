'''
Mariokart project

Copyright 2012 University of Canterbury


Author: Matthew Wigley

This module contains high level functions involving the steering wheel controller in the CANBUS network.

Functions are provided to get the get and set the steering wheel angle.

Note that the UsbController module must be initialized before using this module.

Functions in this module should be accurate to the nearest degree.
'''

import UsbController
import definitions

'''-------------------------
       PRIVATE CONSTANTS
   -------------------------'''

#maximum allowable angle
#TODO: update / fix once steering is calibrated
#not so important in that the logic in the board will make sure that the maximum steering angle is not overshot
#(and will also use a freshly maximum angle value which will be more accurate)
MAX_STEERING_ANGLE = 45 # in degrees

#the angle to be used for standard turning
TURN_ANGLE = 20


'''-------------------------
       PUBLIC FUNCTIONS
   -------------------------'''

#get the steering angle in degrees.  Straight ahead is zero degrees
#left is negative, and right is positive #TODO: check
#returns None if there was an error retrieving the angle
def GetAngle():
    return UsbController.GetValue(definitions.VAR_STEERING_ANGLE)

#set the steering angle in degrees.  Straight ahead is zero degrees
#left is negative, and right is positive #TODO: check
#returns false if setting the angle was not possible
def SetAngle(angle):
    if angle <= MAX_STEERING_ANGLE and angle >= -MAX_STEERING_ANGLE:
        return UsbController.SetValue(definitions.ADDR_STEERING, definitions.VAR_STEERING_ANGLE, angle)
    else:
        return False

#returns the maximum positive or negative angle the steering wheel can be driven to
def GetMaximumAngle():
    return MAX_STEERING_ANGLE

#turn to the left. returns False on failure
def TurnLeft():
    return SetAngle(-TURN_ANGLE)
    

#turn to the right. returns False on failure
def TurnRight():
    return SetAngle(TURN_ANGLE)

#move the steering wheel back to the center position
def TurnStraight():
    return SetAngle(0)

