import time
import board
import busio
import adafruit_htu31d

# Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)

# Create sensor object
sensor = adafruit_htu31d.HTU31D(i2c)

while True:
    temperature, humidity = sensor.temperature, sensor.relative_humidity
    print("Temperature: %0.1f C" % temperature)
    print("Humidity: %0.1f %%" % humidity)
    time.sleep(2)
