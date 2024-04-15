import time
import board
import busio
import adafruit_htu31d
import socket

HEADER = 64 
PORT = 18200
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"
SERVER = socket.gethostbyname(socket.gethostname())
ADDR = (SERVER,PORT)

# Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)

# Create sensor object
sensor = adafruit_htu31d.HTU31D(i2c)

client = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
client.connect(ADDR)

def send(msg):
    message = msg.encode(FORMAT)
    msg_length = len(message)
    send_length = str(msg_length).encode(FORMAT)
    send_length += b' ' * (HEADER - len(send_length))
    #client.send(send_length)
    client.send(message)
    print(client.recv(2048).decode(FORMAT))

temperature, humidity = sensor.temperature, sensor.relative_humidity
    #print("Temperature: %0.1f C" % temperature)
    #print("Humidity: %0.1f %%" % humidity)
currentTemp = f"Temperature: {temperature}, Humidity: {humidity}"
print(temperature,humidity) #If this line can print there's no problem with the temp/humidity sensors
#send("This is a test message! If you can see this, the client is capable of sending messages!")
send(currentTemp)



