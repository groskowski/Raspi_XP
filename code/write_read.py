import spidev
import random
import string
import pigpio
import enum
import json

# Create an SPI object (connecting to /dev/spidev0.1)
spi = spidev.SpiDev(0, 1)
spi.max_speed_hz = 400000

pi = pigpio.pi()
irq_pin = 40

# Send data, d_type must be a number and data must be a byte array
def sendPKG(d_type, data):
    start_byte = "$".encode('utf-8')
    type_byte = bytes([d_type])
    length_byte = bytes([len(data)])
    
    bytes_to_send = start_byte + type_byte + length_byte + data
    
    print(bytes_to_send)
    
    spi.xfer(bytes_to_send)

# Testing Function 
def random_string(length):
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for _ in range(length))

len_data = 250
 
#Radio Test Function
class MessageType(enum.Enum): # Define the MessageType enum
    DATA = 1
    LATCH = 2
    AUDIO_START = 3
    AUDIO_STOP = 4
    ZPOS = 5
    SET_ZPOS = 6
    ZPOW = 7
    CUT = 8
    REBOOT = 9
    PI_ON = 10
    PI_OFF = 11
    TEST = 12
    TEST_PHOTO = 13
    PHOTOS_START = 14
    PHOTOS_STOP = 15
    FAN_START = 16
    FAN_STOP = 17

def random_radio_test():
    # Select a random MessageType
    msg_type = random.choice(list(MessageType))
    # Generate random integers for dest and variable
    dest = random.randint(1, 100)  # Random destination ID
    variable = random.randint(0, 1000)  # Random variable value

    # Return the message as a dictionary
    return json.dumps({"MSG_TYPE": msg_type.name, "dest": dest, "variable": variable}).encode('utf-8')

#sendPKG(1, random_string(len_data).encode('utf-8'))
sendPKG(1, random_radio_test())
 
spi.close()