import time
from MariaDBConnector import DatabaseManager

def execute(command, target):
    print(f"Executing command: {command} on target: {target}")
    # here we'll have an SPI call to execute the command
    return 1

def check_requests():
    while True:
        try:
            db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')

            # Check for unresolved requests
            query = "SELECT * FROM requests WHERE resolved = 0 ORDER BY insertion_date ASC"
            unresolved_requests = db.execute_query(query)

            for request in unresolved_requests:
                request_id = request['id']
                command = request['command']
                target = request['target']
                service_id = request['service_id']

                try:
                    # Execute the requested command
                    status = execute(command, target)

                    # Update the request as resolved
                    update_query = "UPDATE requests SET resolved = 1 WHERE id = %s"
                    db.execute_query(update_query, (request_id,))

                    # Insert a response into the responses table
                    insert_query = "INSERT INTO responses (target, service_id, status) VALUES (%s, %s, %s)"
                    db.execute_query(insert_query, (target, service_id, -1))
                except Exception as e:
                    print(f"Error executing request {request_id}: {str(e)}")
                    # Handle the exception and continue with the next request

            db.close()

        except Exception as e:
            print(f"Error connecting to the database: {str(e)}")
            # Handle the exception and continue with the next iteration

        # Sleep for 500ms before checking again
        time.sleep(0.5)

# Start the request checking service
while True:
    try:
        check_requests()
    except Exception as e:
        print(f"Request checking service crashed: {str(e)}")
        # Handle the exception and restart the service
        time.sleep(1)  # Optional: Add a small delay before restarting