import os
import sys

current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)

from MariaDBConnector import DatabaseManager

from datetime import datetime

class sensor_data:
    __version__ = 1

    def __init__(self, pid, created_at, lat, lon, sat_quality, sats, altitude, temp, humidity, battery1, battery2, battery3, pressure):
        self.insertion_time = datetime.now()
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

db = DatabaseManager(host='localhost', user='pi', password='raspberry', database='ipro497')

obj = sensor_data(1, datetime.now(), 37.7749, -122.4194, 1, 5.1, 100.0, 25.5, 60.0, 3.7, 3.8, 3.9, 1013.25)
db.insert_object(obj)

db.close()