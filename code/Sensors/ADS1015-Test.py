import board
import busio
import adafruit_ads1x15.ads1015 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

# Initialize I2C communication
i2c = busio.I2C(board.SCL, board.SDA)

# Create an instance of the ADS1015 ADC
ads = ADS.ADS1015(i2c)

# Create an analog input channel for AIn0
ain0 = AnalogIn(ads, ADS.P0)

# Define voltage divider constant:
VDIVIDER = (130. + 510.) / 130.

voltage = ain0.voltage * VDIVIDER

print(f"Voltage: {voltage:.3f}V")