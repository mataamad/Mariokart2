from mariokartAPI import UsbController
from mariokartAPI import definitions

from mariokartAPI import brake
from mariokartAPI import steering
from mariokartAPI import speed

import time

def TestSimultaneous():
    steering.SetAngle(15)
    brake.SetPositionMM(15)
    time.sleep(3)
    brake.SetPositionMM(0)
    steering.SetAngle(-10)
    print(speed.GetSpeed())


if __name__ == "__main__":
    UsbController.Open()

    TestSimultaneous()
    UsbController.Close()
