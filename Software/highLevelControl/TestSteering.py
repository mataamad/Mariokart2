from mariokartAPI import UsbController
from mariokartAPI import steering

def testSteeringNegative():
    print("test negative steering angles and zero crossing")
    print(steering.GetAngle())
    print(steering.SetAngle(-15))
    
    while steering.GetAngle() > -10:
        pass
    
    print(steering.GetAngle())
    steering.SetAngle(-20)

    while steering.GetAngle() > -15:
        pass

    print(steering.GetAngle())
    steering.SetAngle(-5)

    while steering.GetAngle() > -7:
        pass

    print("steering angle")
    print(steering.GetAngle())
    print("turning straight")
    print (steering.TurnStraight())


    angle = steering.GetAngle()
    while angle < -4 or angle > 4:
        angle = steering.GetAngle()
        #print("in loop")
        #print(  )

    print('straight angle:')
    print(steering.GetAngle())


    steering.SetAngle(20)
    while angle < 15:
        angle = steering.GetAngle()
        #print("in loop")
        #print(  )

    print("steering angle:")
    print(steering.GetAngle())



def testSteering():
    print("test steering")

    print(steering.SetAngle(0))
    print("zero angle:")
    print(steering.GetAngle())
    print(steering.SetAngle(15))
    
    while steering.GetAngle() < 10:
        pass
    
    print("angle:")
    print(steering.GetAngle())
    steering.SetAngle(20)

    while steering.GetAngle() < 15:
        pass

    print(steering.GetAngle())
    steering.SetAngle(10)

    while steering.GetAngle() > 12:
        pass

    print("angle")
    print(steering.GetAngle())
    print("turn straight command:")
    print (steering.TurnStraight())

    angle = steering.GetAngle()
    while angle < -2 or angle > 2:
        angle = steering.GetAngle()
        #print("in loop")
        #print(  )

    print('straight angle:')
    print(steering.GetAngle())
    print("done test steering")

if __name__ == "__main__":
    UsbController.Open()
    testSteering()
    testSteeringNegative()
    UsbController.Close()
