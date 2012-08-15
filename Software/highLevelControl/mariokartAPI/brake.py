'''
Mariokart project

Copyright 2012 University of Canterbury


Author: Matthew Wigley

This module contains high level functions involving the brake controller in the CANBUS network.

Functions are provided to get the get and set the brake position.

Note that the UsbController module must be initialized before using this module.

Of interest to the end user is:

GetPositionMM
SetPositionMM
SetFullOn
SetOff
GetFullOnPositionMM

Accuracy is at BEST to 101.6/1024 (i.e ~ 0.1mm), and likely less than this.

In particular attempting to loop setting the position of the brake pedal until it is at that exact position
will generally produce an infinite loop.  Instead a tolerance will have to be introduced.
e.g:
    brake.SetPositionMM(30)
    brakePosition = brake.GetPositionMM()
    while brakePosition < 28 or brakePosition > 32:
        brakePosition = brake.GetPositionMM()

will terminate correctly once the brake is near its desired position

'''

import UsbController
import definitions


'''-------------------------
       PRIVATE CONSTANTS
   -------------------------'''
ACTUATOR_LENGTH_MM = 101.6
#size of the ADC, brake goes most of the way to this value - but not all the way.  It should be close enough.
BRAKE_LENGTH_ADC_UNITS = 1024
#the position of the brake when it is 'full off' in ADC units
#TODO: find the exact correct value
BRAKE_OFF_POSITION_ADC_UNITS = 600
#the position of the brake when it is 'full on' in ADC units
#TODO: decide on / find the exact value
BRAKE_FULL_ON_POSITION_ADC_UNITS = 950

#travel from the 'full off' to 'full on' position
BRAKE_TRAVEL_ADC_UNITS = BRAKE_FULL_ON_POSITION_ADC_UNITS - BRAKE_OFF_POSITION_ADC_UNITS


'''-------------------------
       PRIVATE FUNCTIONS
   -------------------------'''

#internal use only - convert a measurement in mm to ADC units
def ConvertMMToADCUnits(mm):
    return (mm*BRAKE_LENGTH_ADC_UNITS)/ACTUATOR_LENGTH_MM

#internal only - convert a measurement in ADC units to mm
def ConvertADCUnitsToMM(adcUnit):
    return (adcUnit*ACTUATOR_LENGTH_MM)/BRAKE_LENGTH_ADC_UNITS


#internal use only - returns the absolute position of the brake actuator in ADC units
#by making a USB request
def GetActuatorPositionADCUnits():
    return UsbController.GetValue(definitions.VAR_BRK_POS)

#internal use only - set the actuator position to an ADC based distance
def SetActuatorPositionADCUnits(position):
    if position >= 0 and position <= BRAKE_LENGTH_ADC_UNITS:
        return UsbController.SetValue(definitions.ADDR_BRAKE, definitions.VAR_BRK_POS, position)
    else:
        return False


'''-------------------------
       PUBLIC FUNCTIONS
   -------------------------'''

#get the current brake position in millimeters from the 'full off' brake position
def GetPositionMM():
    return ConvertADCUnitsToMM(GetActuatorPositionADCUnits() - BRAKE_OFF_POSITION_ADC_UNITS)

#Set the position of the break relative to the brake 'full off' position
def SetPositionMM(positionMM):
    absolutePositionADCUnits = ConvertMMToADCUnits(positionMM) + BRAKE_OFF_POSITION_ADC_UNITS
    if absolutePositionADCUnits >= BRAKE_OFF_POSITION_ADC_UNITS and absolutePositionADCUnits <= BRAKE_FULL_ON_POSITION_ADC_UNITS:
        return SetActuatorPositionADCUnits(absolutePositionADCUnits)
    else:
        return False
       
#set the brake position as a percentage of fully on 
def SetPositionPercent(positionPercent):
    absolutePositionADCUnits = positionPercent*BRAKE_TRAVEL_ADC_UNITS + BRAKE_OFF_POSITION_ADC_UNITS
    if absolutePositionADCUnits >= BRAKE_OFF_POSITION_ADC_UNITS and absolutePositionADCUnits <= BRAKE_FULL_ON_POSITION_ADC_UNITS:
        return SetActuatorPositionADCUnits(absolutePositionADCUnits)
    else:
        return False


#Set the brake to be fully on
def SetFullOn():
    return SetActuatorPositionADCUnits(BRAKE_FULL_ON_POSITION_ADC_UNITS)

#set the brake to be fully off
def SetOff():
    return SetActuatorPositionADCUnits(BRAKE_OFF_POSITION_ADC_UNITS)

#returns the fully on position of the brake in mm (note that the full off position is defined as 0 mm)
def GetFullOnPositionMM():
    return ConvertADCUnitsToMM(BRAKE_TRAVEL_ADC_UNITS)
    


