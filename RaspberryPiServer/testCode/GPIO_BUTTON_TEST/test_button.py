import RPi.GPIO as GPIO  
from time import sleep     # this lets us have a time delay (see line 15)  
GPIO.setmode(GPIO.BCM)     # set up BCM GPIO numbering  
GPIO.setup(5, GPIO.IN, pull_up_down=GPIO.PUD_UP)    # set GPIO25 as input (button)  
#GPIO.setup(24, GPIO.OUT)   # set GPIO24 as an output (LED)  
  
try:  
    while True:            # this will carry on until you hit CTRL+C  
        if GPIO.input(5): # if port 25 == 1  
            #print("Port 25 is 0/LOW/False - LED OFF")
            pass
        else:  
            print("Port 25 is 1/HIGH/True - LED ON")
        sleep(0.1)         # wait 0.1 seconds  
  
finally:                   # this block will run no matter how the try block exits  
    GPIO.cleanup()         # clean up after yourself 
