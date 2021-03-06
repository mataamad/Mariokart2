from mariokartAPI import UsbController
from mariokartAPI import speed
import time

def testSpiMany():
    print("running SPI acceleration test")
    for i in range(1, 255/2):
        speed.SetAcceleration(i*2)
        time.sleep(0.1)
    print("done")


if __name__ == "__main__":
    UsbController.Open()
        
    testSpiMany()

    UsbController.Close()
