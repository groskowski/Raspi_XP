import time
import logging
from database import get_db_connection
from spi_comms import SPIComs, wrap_command

def execute(command: str, target: int, coms: SPIComs):
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
            send_command(wrap_command(command, target, value.strip()), coms)
        else:
            send_command(wrap_command(command, target), coms)
        time.sleep(1) # Some sleep time to make sure the CRC doesn't die
        
        return 1 # Success
    except Exception as e:
        logging.error(f"Error executing command: {str(e)}")
        time.sleep(1)

        return 0 # Failure

def send_command(command, coms=None):
    try:
        if coms == None:
            coms = SPIComs()
        coms.send_pkg(coms.get_pkg(1, command))
    except Exception as e:
        logging.error(f"Error using SPI: {str(e)}")

def check_requests(coms=None):
    """
    Check for unresolved requests in the database and execute them.
    """

    db = None

    while True:
        try:

            db = get_db_connection(db)
            
            # Finds unresolved requests
            query = "SELECT * FROM requests WHERE resolved = 0 ORDER BY insertion_date ASC"
            unresolved_requests = db.execute_query(query)

            # Tries to resolve them
            for request in unresolved_requests:

                print(request)
                request_id, _, command, target, service_id, _ = request
                
                try:
                    if execute(command, target, coms):
                        # If success, marks request as resolved
                        update_query = "UPDATE requests SET resolved = 1 WHERE id = %s"
                        db.execute_query(update_query, (request_id,))
                        
                        # And proceeds to create a response instance in the database
                        insert_query = "INSERT INTO responses (target, service_id, status) VALUES (%s, %s, %s)"
                        db.execute_query(insert_query, (target, service_id, -1))

                except Exception as e:
                    logging.error(f"Error executing request {request_id}: {str(e)}")
                    db.connection.rollback()

        except Exception as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            db = None
            coms.close()
            time.sleep(0.5)
            coms = SPIComs()