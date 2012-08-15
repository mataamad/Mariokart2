from mariokartAPI import UsbController
from mariokartAPI import speed
import time

def testSpiMany():
    print("running SPI acceleration test")
    for i in range(1, 255):
        speed.SetAcceleration(i)
        time.sleep(0.1)
    print("done")

UsbController.Open()
    
testSpiMany()

UsbController.Close()
