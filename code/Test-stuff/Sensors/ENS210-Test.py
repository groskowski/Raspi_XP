import RPi.GPIO as GPIO
import smbus
import time

EN_POW_PIN = 33
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)
GPIO.setup(EN_POW_PIN, GPIO.OUT)
GPIO.output(EN_POW_PIN, GPIO.HIGH)
time.sleep(.15)

# Create an SMBus instance
bus = smbus.SMBus(1)

# ENS210 I²C address (check with i2cdetect)
address = 0x43

# Registers
REG_SENS_START = 0x21
REG_T_VAL = 0x30
REG_H_VAL = 0x33

# Function to read temperature and humidity
def read_ens210():
    # Start a single shot measurement
    bus.write_byte_data(address, REG_SENS_START, 0x03)
    # Wait for measurement to complete
    time.sleep(0.2)  # 200ms for safety

    # Read temperature and humidity values
    t_val = bus.read_i2c_block_data(address, REG_T_VAL, 3)
    h_val = bus.read_i2c_block_data(address, REG_H_VAL, 3)

    # Convert the read values
    t_raw = (t_val[2] << 16) | (t_val[1] << 8) | t_val[0]
    h_raw = (h_val[2] << 16) | (h_val[1] << 8) | h_val[0]

    # Extract valid data (first 16 bits) and convert
    t_data = t_raw & 0xFFFF
    h_data = h_raw & 0xFFFF

    # Convert raw temperature to Celsius (assuming no solder correction)
    temperature = (t_data / 64.0) - 273.15

    # Convert raw humidity to percentage
    humidity = h_data / 512.0

    return temperature, humidity

# Main loop to read the sensor data
while True:
    temperature, humidity = read_ens210()
    print(f"Temperature: {temperature:.2f} C, Humidity: {humidity:.2f} %")
    time.sleep(1)  # Delay between readings
