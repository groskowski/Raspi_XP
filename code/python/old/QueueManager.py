import time
from MariaDBConnector import DatabaseManager, LoggingHandler
import logging, json
from spi_comms import SPIComs

db = None

def get_db_connection():
    """
    Get the database connection. If no connection exists, create a new one.

    :return: The DatabaseManager instance.
    """
    global db
    if db is None:
        try:
            db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')
        except Exception as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            raise
    return db

logger = logging.getLogger()
logger.setLevel(logging.INFO)
handler = LoggingHandler(get_db_connection())
logger.addHandler(handler)
coms = SPIComs()

def wrap_command(command, target, param=0): return json.dumps({"MSG_TYPE": command, "dest": target, "variable": param}).encode('utf-8')
def send_command(command): coms.send_pkg(coms.get_pkg(1, command))

def execute(command: str, target: int):
    """
    Execute a command on a target node.

    :param command: The command to execute.
    :param target: The target node ID.
    :return: The status of the command execution.
    """
    try:
        logging.info(f'Sending command {command} to node #{target}')
        if ';' in command:
            command, value = command.split(';')
            send_command(wrap_command(command, target, value.strip()))
        else: send_command(wrap_command(command, target))
        time.sleep(1)
        return 1
    except Exception as e:
        logging.error(f"Error executing command: {str(e)}")
        time.sleep(1)
        return 0

def check_requests():
    """
    Check for unresolved requests in the database and execute them.
    """
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
                    if execute(command, target):
                        update_query = "UPDATE requests SET resolved = 1 WHERE id = %s"
                        db.execute_query(update_query, (request_id,))
                        insert_query = "INSERT INTO responses (target, service_id, status) VALUES (%s, %s, %s)"
                        db.execute_query(insert_query, (target, service_id, -1))
                except Exception as e:
                    logging.error(f"Error executing request {request_id}: {str(e)}")
                    db.connection.rollback()
        except Exception as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            db = None
            time.sleep(0.5)
            coms.close()
            time.sleep(0.5)
            coms = SPIComs()

while True:
    try:
        check_requests()
    except Exception as e:
        logging.error(f"Request checking service crashed: {str(e)}")