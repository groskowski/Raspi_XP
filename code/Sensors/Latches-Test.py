import RPi.GPIO as GPIO
from time import sleep

EN_POW_PIN = 15
LA_PIN = 37
LB_PIN = 29
LC_PIN = 22

# Set up the GPIO mode (BOARD or BCM)
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

# Set pins as an output
GPIO.setup(EN_POW_PIN, GPIO.OUT)
GPIO.output(EN_POW_PIN, GPIO.LOW)

GPIO.setup(LA_PIN, GPIO.OUT)
GPIO.output(LA_PIN, GPIO.LOW)

GPIO.setup(LB_PIN, GPIO.OUT)
GPIO.output(LB_PIN, GPIO.LOW)

GPIO.setup(LC_PIN, GPIO.OUT)
GPIO.output(LC_PIN, GPIO.LOW)

GPIO.output(EN_POW_PIN, GPIO.HIGH)
sleep(.15)

del_c = 0.5
GPIO.output(LA_PIN, GPIO.HIGH)
sleep(del_c)
GPIO.output(LB_PIN, GPIO.HIGH)
sleep(del_c)
GPIO.output(LC_PIN, GPIO.HIGH)

sleep(.25)

GPIO.output(EN_POW_PIN, GPIO.LOW)
GPIO.output(LA_PIN, GPIO.LOW)
GPIO.output(LB_PIN, GPIO.LOW)
GPIO.output(LC_PIN, GPIO.LOW)