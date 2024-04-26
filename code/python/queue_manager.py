import time, io
from PIL import Image
import logging
from database import get_db_connection
from spi_comms import SPIComs, wrap_command
import traceback

def wait_for_package(coms, timeout=30):
    start_time = time.time()
    while not coms.read_irq_pin():
        if time.time() - start_time > timeout:
            raise TimeoutError("Timeout waiting for response")
    message = coms.read_pkg()
    delta_t = time.time() - start_time
    logging.info(f"Received message: {message} after {delta_t}s")
    return message, delta_t

def reconstruct_image(chunks):
    binary_data = b''.join(chunks)
    image = Image.open(io.BytesIO(binary_data))
    return image

def request_chunk(coms, target, chunk_num):
    send_command(f'REQUEST_CHUNK;{chunk_num}', target=target, coms=coms)
    message, _ = wait_for_package(coms)
    return message

def insert_sensor_data(message, pid, db):
    try:
        print(message)
        values = message.split(",")
        
        sql = """
            INSERT IGNORE INTO sensor_data (
                created_at, lat, lon, sat_quality, sats, altitude, temp, humidity,
                battery1, battery2, battery3, pressure, insertion_time, pid
            )
            VALUES (
                {}, {}, {}, {}, {}, {}, {}, {},
                {}, {}, {}, {}, NOW(), {}
            )
        """.format(
            "NULL" if values[0] == "N/A" else "'" + str(values[0]) + "'",
            "NULL" if values[1] == "N/A" else "'" + str(values[1]) + "'",
            "NULL" if values[2] == "N/A" else "'" + str(values[2]) + "'",
            "NULL" if values[3] == "N/A" else "'" + str(values[3]) + "'",
            "NULL" if values[4] == "N/A" else "'" + str(values[4]) + "'",
            "NULL" if values[5] == "N/A" else "'" + str(values[5]) + "'",
            "NULL" if values[6] == "N/A" else "'" + str(values[6]) + "'",
            "NULL" if values[7] == "N/A" else "'" + str(values[7]) + "'",
            "NULL" if values[8] == "N/A" else "'" + str(values[8]) + "'",
            "NULL" if values[9] == "N/A" else "'" + str(values[9]) + "'",
            "NULL" if values[10] == "N/A" else "'" + str(values[10]) + "'",
            "NULL" if values[11] == "N/A" else "'" + str(values[11]) + "'",
            pid
        )
        print(sql)
        db.execute_query(sql.strip())
        print("Sensor data inserted successfully.")

    except Exception as error:
        print(f"Error inserting sensor data: {error}")

def execute(command: str, target: int, coms: SPIComs, service_id=None):
    """
    Execute a command on a target node.

    :param command: The command to execute.
    :param target: The target node ID.
    :return: The status of the command execution.
    """
    try:
        send_command(command, target, coms)
        if 'TEST_PHOTO' in command:
            message, time = wait_for_package(coms)

            n = int(str(message.decode('utf-8', 'ignore')[3:]))
            print(f'number of chunks: {n}')
                            
            chunks = [None] * n
            requested_chunks = set()

            max_requests = 2 * n
            num_requests = 0

            while len(requested_chunks) < n and num_requests < max_requests:
                print(requested_chunks, num_requests, n, max_requests)
                for i in range(n):
                    if chunks[i] is None and i not in requested_chunks:
                        print(f'requesting chunk {i + 1}')
                        chunk = request_chunk(coms, target, i+1)[3:]

                        chunks[i] = chunk
                        requested_chunks.add(i)
                        num_requests += 1

                for i in range(n):
                    if chunks[i] is None and num_requests < max_requests:
                        print(f'requesting chunk {i +1}')
                        chunk = request_chunk(coms, target, i+1)[3:]
                        chunks[i] = chunk
                        num_requests += 1

            if None in chunks:
                raise TimeoutError("Failed to receive all chunks within the request limit")
            
            image = reconstruct_image(chunks)
            image.save(f"/home/pi/code/Webserver/img/node_{int(target)-10}.jpg") 

        else: message, time = wait_for_package(coms)

        return 1, message  # Success

    except TimeoutError:
        logging.error("Timeout waiting for response")
        print(traceback.format_exc())
        return 0, None  # Failure

    except Exception as e:
        logging.error(f"Error executing command: {str(e)}")
        print(traceback.format_exc())
        return 0, None  # Failure

def send_command(command, target, coms=None):
    logging.info(f'Sending command {command} to node #{target}')
    try:
        if coms == None: coms = SPIComs()
        coms.send_pkg(coms.get_pkg(1, wrap_command(command.split(';')[0] if ';' in command else command, 
                                                   target, 
                                                   param=(command.split(';')[1].strip() if ';' in command else 0))))
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
                    result, response = execute(command, str(int(target) + 10), coms, service_id)
                    print('finished requesting')
                    update_query = "UPDATE requests SET resolved = 1 WHERE id = %s"
                    db.execute_query(update_query, (request_id,))         

                    if 'DATA' in command and response:
                        insert_sensor_data(response[3:].decode('utf-8'), target, db)

                    if result:
                        status, message = result 
                        insert_query = "INSERT INTO responses (target, service_id, status, response) VALUES (%s, %s, %s, %s)"
                        db.execute_query(insert_query, (target, service_id, 1, message))
                    else:
                        insert_query = "INSERT INTO responses (target, service_id, status, response) VALUES (%s, %s, %s, %s)"
                        db.execute_query(insert_query, (target, service_id, -1, None))
                except Exception as e:
                    logging.error(f"Error executing request {request_id}: {str(e)}")
                    db.connection.rollback()

        except Exception as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            db = None
            coms.close()
            time.sleep(0.5)
            coms = SPIComs()