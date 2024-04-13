import os
import sys
from datetime import datetime, timedelta
import random

current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)
from MariaDBConnector import DatabaseManager

class node_table:
    __version__ = 1

    def __init__(self, pid, type, address):
        self.insertion_time = datetime.now()
        self.pid = pid
        self.type = type
        self.address = address
        self.last_update = datetime.now()

# Generate and insert 20 nodes into the database
db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')
num_nodes = 20

for pid in range(1, num_nodes + 1):
    if pid <= num_nodes * 0.2:
        type = "special"
    elif pid <= num_nodes * 0.5:
        type = "shared"
    else:
        type = "sensor"

    address = f"node_{pid}"

    # Create a node_table object with the generated data
    node = node_table(pid, type, address)

    # Insert the object into the database
    db.insert_object(node)

db.close()
print("Nodes inserted successfully.")