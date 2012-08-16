from mariokartAPI import UsbController
from mariokartAPI import definitions

from mariokartAPI import brake
from mariokartAPI import steering
from mariokartAPI import speed

import TestBrake
import TestSteering
import TestSpeedSensor
import TestSpi
import TestError
import TestSimul

import time

#tests were run on import but lets run them again (along with all other ones)
UsbController.Open()
UsbController.WaitUntilRunning()
print("boards are in run state")

print("steering")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestSteering.testSteering()
    print("steering negative")
    TestSteering.testSteeringNegative()

print("brake")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestBrake.testBrake()
print("speed sensor")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestSpeedSensor.testSpeedSensor()
print("spi")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestSpi.testSpiMany()
print("simultaneous")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestSimul.TestSimultaneous()
print("error")
var = raw_input("do test?: (Y)")
if var == "Y" or var == "y":
    TestError.TestErrorTransition()

UsbController.Close()

