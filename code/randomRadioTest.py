from SPIComs import *
import enum
import json
import random

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
coms = SPIComs()
coms.send_pkg(coms.get_pkg(1, random_radio_test()))
coms.close()