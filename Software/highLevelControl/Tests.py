from mariokartAPI import UsbController
from mariokartAPI import definitions

from mariokartAPI import brake
from mariokartAPI import steering
from mariokartAPI import speed

import TestBrake
import TestSteering
import TestSpeedSensor
import TestSpi
import TestSimul

import time


def testLowLevelGetSet():
    print("getting brake value")
    print(UsbController.GetValue(definitions.VAR_BRK_POS))
    print("done")

    print("testing setting of brake position")
    UsbController.SetValue(definitions.ADDR_BRAKE, definitions.VAR_BRK_POS, 1024)
    print("done testing setting brake position")

    print("steering angle:")
    print(UsbController.GetValue(definitions.VAR_STEERING_ANGLE))
    UsbController.SetValue(definitions.ADDR_STEERING, definitions.VAR_STEERING_ANGLE, 20)


#tests were run on import but lets run them again (along with all other ones)
UsbController.Open()
UsbController.WaitUntilRunning()
print("boards are in run state")


print("steering")
TestSteering.testSteering()
print("steering negative")
TestSteering.testSteeringNegative()
print("brake")
TestBrake.testBrake()
print("speed sensor")
TestSpeedSensor.testSpeedSensor()
print("spi")
TestSpi.testSpiMany()
print("simultaneous")
TestSimul.TestSimultaneous()

UsbController.Close()




