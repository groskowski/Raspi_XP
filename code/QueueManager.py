import time
from MariaDBConnector import DatabaseManager, LoggingHandler
import logging

db = None

def get_db_connection():
    global db
    if db is None:
        db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')
    return db

logger = logging.getLogger()
logger.setLevel(logging.INFO)
handler = LoggingHandler(get_db_connection())
logger.addHandler(handler)


def execute(command, target):
    logging.info(f'Sending command {command} to node #{target}')
    time.sleep(2)
    return 1

def check_requests():
    global db
    while True:
        try:
            db = get_db_connection()
            query = "SELECT * FROM requests WHERE resolved = 0 ORDER BY insertion_date ASC"
            unresolved_requests = db.execute_query(query)
            for request in unresolved_requests:
                request_id = request[0]
                time_ = request[1]
                command = request[2]
                target = request[3]
                service_id = request[4]
                try:
                    status = execute(command, target)
                    update_query = "UPDATE requests SET resolved = 1 WHERE id = %s"
                    db.execute_query(update_query, (request_id,))
                    insert_query = "INSERT INTO responses (target, service_id, status) VALUES (%s, %s, %s)"
                    db.execute_query(insert_query, (target, service_id, -1))
                except Exception as e:
                    print(f"Error executing request {request_id}: {str(e)}")
        except Exception as e:
            print(f"Error connecting to the database: {str(e)}")
            db = None
            time.sleep(0.5)

while True:
    try:
        check_requests()
    except Exception as e:
        print(f"Request checking service crashed: {str(e)}")