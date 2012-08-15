from mariokartAPI import UsbController
from mariokartAPI import brake
import time

def testBrake():
    print("brake test")
     #testing some of brake.py
    print("brake position in mm:")
    print(brake.GetPositionMM())
    print("brake position in adc:")
    print(brake.GetActuatorPositionADCUnits())

    brake.SetPositionMM(20)

    time.sleep(5)
    print("brake position in mm:")
    print(brake.GetPositionMM())

    brake.SetPositionMM(0)

    time.sleep(5)

    brake.SetFullOn()

    #brake.SetActuatorPositionADCUnits(0)
    print("done brake test")

UsbController.Open()
testBrake()
UsbController.Close()
