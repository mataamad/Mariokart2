from mariokartAPI import UsbController
from mariokartAPI import brake
import time

def testBrake():
    print("brake test")
     #testing some of brake.py
    print("initial brake position in mm:")
    print(brake.GetPositionMM())
    print("initial brake position in adc:")
    print(brake.GetActuatorPositionADCUnits())


    print("moving brake to 20mm")
    brake.SetPositionMM(20)

    time.sleep(5)
    print("brake position in mm:")
    print(brake.GetPositionMM())

    print("moving brake to 0mm")
    brake.SetPositionMM(0)

    time.sleep(5)

    print("brake position in mm:")
    print(brake.GetPositionMM())


    print("setting brake to full on")
    brake.SetFullOn()

    print("done brake test")

if __name__ == "__main__":
    UsbController.Open()
    testBrake()
    UsbController.Close()
