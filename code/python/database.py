import logging
import mysql.connector  
from datetime import datetime

class DatabaseManager:
    def __init__(self, host: str, user: str, password: str, database: str):
        """
        Initialize the DatabaseManager with the given database connection parameters.

        :param host: The hostname of the database server.
        :param user: The username for the database connection.
        :param password: The password for the database connection.
        :param database: The name of the database to connect to.
        """
        try:
            self.connection = mysql.connector.connect(
                host=host,
                user=user,
                password=password,
                database=database
            )
            logging.info("Connected to the database successfully")
            print("Connected to the database successfully")
        except mysql.connector.Error as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            raise

    def insert_object(self, obj: object):
        """
        Insert an object into the corresponding table in the database.

        :param obj: The object to be inserted.
        """
        cursor = None
        try:
            table_name = type(obj).__name__
            attributes = vars(obj)
            attributes.pop('__version__', None)  # Remove the '__version__' attribute if present

            table_exists_query = f"SHOW TABLES LIKE '{table_name}'"
            table_exists = self.execute_query(table_exists_query)

            if not table_exists:
                self.create_table(table_name, attributes)
            else:
                describe_query = f"DESCRIBE {table_name}"
                existing_columns = [column[0] for column in self.execute_query(describe_query)]
                missing_columns = set(attributes.keys()) - set(existing_columns)

                for column in missing_columns:
                    self.add_column(table_name, column, attributes[column])

            columns = ', '.join(attributes.keys())
            placeholders = ', '.join(['%s'] * len(attributes))
            insert_query = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
            self.execute_query(insert_query, list(attributes.values()))
            logging.info(f"Inserted object into {table_name} table")
        except Exception as e:
            logging.error(f"Error inserting object: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def create_table(self, table_name: str, attributes: dict):
        """
        Create a new table in the database based on the given table name and attributes.

        :param table_name: The name of the table to be created.
        :param attributes: A dictionary representing the attributes of the table.
        """
        cursor = None
        try:
            column_definitions = []
            for column, value in attributes.items():
                if isinstance(value, int):
                    column_definitions.append(f"{column} INT")
                elif isinstance(value, float):
                    column_definitions.append(f"{column} FLOAT")
                elif isinstance(value, str):
                    column_definitions.append(f"{column} VARCHAR(255)")
                elif isinstance(value, datetime):
                    column_definitions.append(f"{column} DATETIME")

            column_definitions.insert(0, "id INT AUTO_INCREMENT PRIMARY KEY")
            create_query = f"CREATE TABLE {table_name} ({', '.join(column_definitions)})"
            self.execute_query(create_query)
            logging.info(f"Created table {table_name}")
        except Exception as e:
            logging.error(f"Error creating table: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def execute_query(self, query: str, params=None):
        """
        Execute a SQL query with optional parameters.
        :param query: The SQL query to be executed.
        :param params: Optional parameters to be passed to the query.
        :return: The result of the query.
        """
        cursor = None
        try:
            cursor = self.connection.cursor()
            cursor.execute(query, params)
            result = cursor.fetchall()
            self.connection.commit()
            return result
        except mysql.connector.Error as e:
            logging.error(f"Error executing query: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def add_column(self, table_name: str, column: str, value: object):
        """
        Add a new column to the specified table based on the given column name and value.

        :param table_name: The name of the table to add the column to.
        :param column: The name of the new column.
        :param value: The value to determine the data type of the new column.
        """
        cursor = None
        try:
            if isinstance(value, int):
                alter_query = f"ALTER TABLE {table_name} ADD COLUMN {column} INT"
            elif isinstance(value, float):
                alter_query = f"ALTER TABLE {table_name} ADD COLUMN {column} FLOAT"
            elif isinstance(value, str):
                alter_query = f"ALTER TABLE {table_name} ADD COLUMN {column} VARCHAR(255)"
            elif isinstance(value, datetime):
                alter_query = f"ALTER TABLE {table_name} ADD COLUMN {column} DATETIME"
            self.execute_query(alter_query)
            logging.info(f"Added column {column} to table {table_name}")
        except Exception as e:
            logging.error(f"Error adding column: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def query_object(self, cls: type, **kwargs):
        """
        Query an object from the database based on the given class and keyword arguments.

        :param cls: The class of the object to be queried.
        :param kwargs: Keyword arguments representing the conditions for the query.
        :return: The queried object if found, None otherwise.
        """
        cursor = None
        try:
            table_name = cls.__name__
            conditions = ' AND '.join([f"{column} = %s" for column in kwargs.keys()])
            query = f"SELECT * FROM {table_name} WHERE {conditions}"
            result = self.execute_query(query, list(kwargs.values()))

            if result:
                obj = cls()
                attributes = vars(obj)
                for column, value in zip(attributes.keys(), result[0]):
                    if column != '__version__':
                        setattr(obj, column, value)
                return obj
            else:
                logging.warning(f"No object found in table {table_name} with the given conditions")
                return None
        except Exception as e:
            logging.error(f"Error querying object: {str(e)}")
            raise
        finally:
            if cursor:
                cursor.close()

    def delete_object(self, cls: type, **kwargs):
        """
        Delete an object from the database based on the given class and keyword arguments.

        :param cls: The class of the object to be deleted.
        :param kwargs: Keyword arguments representing the conditions for the deletion.
        """
        cursor = None
        try:
            table_name = cls.__name__
            conditions = ' AND '.join([f"{column} = %s" for column in kwargs.keys()])
            delete_query = f"DELETE FROM {table_name} WHERE {conditions}"
            self.execute_query(delete_query, list(kwargs.values()))
            logging.info(f"Deleted object from table {table_name}")
        except Exception as e:
            logging.error(f"Error deleting object: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def update_object(self, obj: object):
        """
        Update an object in the database.

        :param obj: The object to be updated.
        """
        cursor = None
        try:
            table_name = type(obj).__name__
            attributes = vars(obj)
            attributes.pop('__version__', None)  # Remove the '__version__' attribute if present

            update_statements = ', '.join([f"{column} = %s" for column in attributes.keys()])
            values = list(attributes.values()) + [obj.id]
            update_query = f"UPDATE {table_name} SET {update_statements} WHERE id = %s"
            self.execute_query(update_query, values)
            logging.info(f"Updated object in table {table_name}")
        except Exception as e:
            logging.error(f"Error updating object: {str(e)}")
            self.connection.rollback()
            raise
        finally:
            if cursor:
                cursor.close()

    def close(self):
        """
        Close the database connection.
        """
        try:
            self.connection.close()
            logging.info("Closed the database connection")
            print("Closed the database connection")
        except Exception as e:
            logging.error(f"Error closing the database connection: {str(e)}")
            raise

def get_db_connection(db=None):
    """
    Get the database connection. If no connection exists, create a new one.

    :return: The DatabaseManager instance.
    """
    if db == None:
        try:
            db = DatabaseManager(host='localhost', user='root', password='raspberry', database='ipro497')
        except Exception as e:
            logging.error(f"Error connecting to the database: {str(e)}")
            raise
    return db