import asyncio
import websockets
import os
import sys
from datetime import datetime, timedelta
import random

current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)

from MariaDBConnector import DatabaseManager


# ! ----
TOTAL_NODES = 20 # ? perhaps a list with the actual node addresses? 
# ! ----


class sensor_data:
    __version__ = 1

    def __init__(self, pid, created_at, lat, lon, sat_quality, sats, altitude, temp, humidity, battery1, battery2, battery3, pressure):
        self.insertion_time = datetime.now() + timedelta(seconds=random.randint(1, 300))  # Add random delay
        self.created_at = created_at
        self.lat, self.lon = lat, lon
        self.pid = pid
        self.sat_quality = sat_quality
        self.sats = sats
        self.altitude = altitude
        self.temp = temp
        self.humidity = humidity
        self.battery1 = battery1
        self.battery2 = battery2
        self.battery3 = battery3
        self.pressure = pressure

connected_clients = []

async def handle_websocket(websocket, path):
    connected_clients.append(websocket)
    try:
        async for message in websocket:
            print(f"Received message: {message}")
            if message == "REQUEST_SENSOR_DATA":
                for i in range(TOTAL_NODES):
                    # ! you will have to uncomment all the comments below that have ? in them 
                    # ? db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')
                   
                    # Tururu put the sensor data acquisition thing here
    
                    # ! talks to the arduino and shoves the sensor data in the database
                    # some code

                    # ! shove it into an object
                    # ? data = sensor_data(pid, created_at, lat, lon, sat_quality, sats, altitude, temp, humidity, battery1, battery2, battery3, pressure)
                    # Tururu put the sensor data acquisition thing here

                    # ? db.insert_object(data)
                    # ? db.close()

                    progress = (i + 1) / TOTAL_NODES * 100
                    await websocket.send(f"Progress: {progress:.0f}%")
                    await asyncio.sleep(0.5)
    finally:
        connected_clients.remove(websocket)

start_server = websockets.serve(handle_websocket, "localhost", 7000)
print("WebSocket server started on port 7000")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()