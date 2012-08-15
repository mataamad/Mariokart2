from mariokartAPI import UsbController
from mariokartAPI import definitions

from mariokartAPI import brake
from mariokartAPI import steering
from mariokartAPI import speed


UsbController.Open()
    
UsbController.SetValue('\x07', '\x10', 1024)

UsbController.Close()
