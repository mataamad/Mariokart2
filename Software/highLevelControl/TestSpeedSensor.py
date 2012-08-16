from mariokartAPI import UsbController
from mariokartAPI import speed
import time


def testSpeedSensor():
    for i in range(1, 20):
        print(speed.GetSpeed())
        time.sleep(0.5)


if __name__ == "__main__":
    UsbController.Open()
    testSpeedSensor()

    UsbController.Close()
