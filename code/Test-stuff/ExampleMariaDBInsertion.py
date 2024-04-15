import os
import sys
from datetime import datetime, timedelta
import random

current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)
from MariaDBConnector import DatabaseManager

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

db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')

num_sensors = 20
entries_per_sensor = 5

for sensor_id in range(1, num_sensors + 1):
    for entry in range(entries_per_sensor):
        # Generate random data for the sensor
        created_at = datetime.now() - timedelta(
            days=random.randint(0, 30),
            hours=random.randint(0, 23),
            minutes=random.randint(0, 59),
            seconds=random.randint(0, 59)
        )
        lat = round(random.uniform(-90, 90), 4)
        lon = round(random.uniform(-180, 180), 4)
        pid = sensor_id
        sat_quality = random.randint(0, 5)
        sats = round(random.uniform(0, 10), 1)
        altitude = random.randint(0, 1000)
        temp = round(random.uniform(-20, 40), 1)
        humidity = random.randint(0, 100)
        battery1 = round(random.uniform(3, 4), 1)
        battery2 = round(random.uniform(3, 4), 1)
        battery3 = round(random.uniform(3, 4), 1)
        pressure = round(random.uniform(900, 1100), 2)

        # Create a sensor_data object with the generated data
        obj = sensor_data(pid, created_at, lat, lon, sat_quality, sats, altitude, temp, humidity, battery1, battery2, battery3, pressure)

        # Insert the object into the database
        db.insert_object(obj)

db.close()
print("Data inserted successfully.")