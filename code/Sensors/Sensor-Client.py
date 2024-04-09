import serial
import time
import socket
import board
import busio
import adafruit_ads1x15.ads1015 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

print("This is in fact running...") #debugging

# Network settings
HEADER = 64
PORT = 18200  # Use the same port as your server
SERVER = socket.gethostbyname(socket.gethostname())
ADDR = (SERVER, PORT)
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"

# Establish a socket connection
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)

print("Socket Established...") #debugging

# Helper function to send messages to the server
def send(msg):
    message = msg.encode(FORMAT)
    client.send(message)

# Initialize I2C communication for ADS1015
i2c = busio.I2C(board.SCL, board.SDA)

# Create an instance of the ADS1015 ADC
ads = ADS.ADS1015(i2c)

# Create an analog input channel for AIn0
ain0 = AnalogIn(ads, ADS.P0)

# Define voltage divider constant:
VDIVIDER = (130. + 510.) / 130.

voltage = ain0.voltage * VDIVIDER

print(f"Voltage: {voltage:.3f}V")

# Helper function to convert raw GPS data to DMS format
def to_dms(raw_value, direction):
    # Extract the degrees and the raw minutes
    degrees = int(raw_value) // 100
    raw_minutes = raw_value - (degrees * 100)
    
    # Convert the minutes to seconds (60 seconds = 1 minute)
    minutes = int(raw_minutes)
    seconds = (raw_minutes - minutes) * 60
    
    # Format the output
    dms = f"{degrees}°{minutes}'{seconds:.1f}\"{direction}"
    return dms

# Function to parse GPGGA sentences for GPS information
def parse_gpgga(sentence):
    fields = sentence.split(',')
    try:
        latitude = to_dms(float(fields[2]), fields[3]) if fields[2] else 'N/A'
        longitude = to_dms(float(fields[4]), fields[5]) if fields[4] else 'N/A'
    except ValueError:
        #print("Error converting GPS data. Skipping sentence.") #debugging
        return None
    
    altitude = f"{fields[9]} {fields[10]}" if fields[9] else "N/A"

    gps_data = {
        'time': fields[1],
        'latitude': latitude,
        'longitude': longitude,
        'quality': fields[6],
        'satellites': fields[7],
        'altitude': altitude,
    }
    return gps_data

# Function to format GPS data for printing
def format_gps_data(gps_data):
    # Extract and format the time
    hours, minutes, seconds = gps_data['time'][:2], gps_data['time'][2:4], gps_data['time'][4:]
    formatted_time = f"{hours}:{minutes}:{seconds}"
    
    # Construct the formatted GPS data string
    return f"Time: {formatted_time}, Latitude: {gps_data['latitude']}, Longitude: {gps_data['longitude']}, Quality: {gps_data['quality']}, Satellites: {gps_data['satellites']}, Altitude: {gps_data['altitude']}, Voltage: {voltage:.3f}V"

# Open serial connection default for BE-220 is 38400bps
ser = serial.Serial('/dev/ttyS0', 38400, timeout=1)

try:
    found_gpgga = False
    while not found_gpgga:
        line = ser.readline().decode('utf-8').rstrip()
        if line.startswith('$GNGGA'):  # Process only GNGGA sentences
            gps_data = parse_gpgga(line)
            formatted_gps_data = format_gps_data(gps_data)
            #print(formatted_gps_data)  # Print formatted GPS data #debugging
            send(formatted_gps_data)  # Send formatted GPS data over the network
            found_gpgga = True  # Mark as found and exit loop
        time.sleep(0.5)  # Adjust the sleep time as needed
except KeyboardInterrupt:
    print("Program terminated!") #debugging
finally:
    ser.close()  # Ensure the serial connection is closed
    client.close()  # Close the socket connection