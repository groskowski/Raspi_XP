import random
import datetime
from MariaDBConnector import DatabaseManager

# Create a DatabaseManager instance
db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')

# Create the log table if it doesn't exist
create_table_query = '''
    CREATE TABLE IF NOT EXISTS log (
        id INT AUTO_INCREMENT PRIMARY KEY,
        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        message VARCHAR(255),
        type VARCHAR(20)
    )
'''
db.execute_query(create_table_query)

# Generate and insert random log entries
log_types = ['error', 'warning', 'info']
log_messages = [
    'Server started successfully',
    'Connection established',
    'Data received from sensor',
    'Error in sensor communication',
    'Warning: Low battery level',
    'Data processing completed',
    'Invalid request format',
    'Database connection lost',
    'System update available',
    'Scheduled maintenance started'
]

for _ in range(100):
    log_type = random.choice(log_types)
    log_message = random.choice(log_messages)
    
    # Generate a random timestamp within the last 30 days
    current_time = datetime.datetime.now()
    random_days = random.randint(0, 30)
    random_seconds = random.randint(0, 86400)  # Number of seconds in a day
    random_time = current_time - datetime.timedelta(days=random_days, seconds=random_seconds)
    formatted_time = random_time.strftime('%Y-%m-%d %H:%M:%S')
    
    # Insert the log entry into the database
    insert_query = "INSERT INTO log (time, message, type) VALUES (%s, %s, %s)"
    db.execute_query(insert_query, (formatted_time, log_message, log_type))

print("Random log entries inserted successfully.")

# Close the database connection
db.close()