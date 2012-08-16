#this file tests error.py
#it ensures that the system is not initially in an error state, then uses a usb command to force the system into an error state
#and then tests that the system is in an error state

from mariokartAPI import UsbController

from mariokartAPI import error

def TestErrorTransition():
    assert(error.AreBoardsInErrorState() == False) #precondition, system isn't already in an error state

    UsbController.WaitUntilRunning()
    print("boards are running")


    print("boards in error state:")
    print(error.AreBoardsInErrorState())
    assert(error.AreBoardsInErrorState() == False)


    print("entering error state:")
    error.EnterErrorState()

    print("boards in error state:")
    print(error.AreBoardsInErrorState())
    assert(error.AreBoardsInErrorState() == True)

if __name__ == "__main__":
    UsbController.Open()

    TestErrorTransition()


    UsbController.Close()
