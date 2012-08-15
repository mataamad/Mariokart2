from mariokartAPI import UsbController
from mariokartAPI import definitions

from mariokartAPI import brake
from mariokartAPI import steering
from mariokartAPI import speed

UsbController.Open()
steering.SetAngle(15)
brake.SetPosition(0)
print(speed.GetSpeed())
UsbController.Close()
