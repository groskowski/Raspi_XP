import serial

# Use '/dev/ttyACM0' for the SparkFun Arduino board
ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
    if ser.in_waiting > 0:
        message = ser.readline().decode('utf-8').rstrip()
        print("Received message:", message)