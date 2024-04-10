import mysql.connector
from time import time
from datetime import datetime

class DatabaseManager:
    def __init__(self, host, user, password, database):
        self.connection = mysql.connector.connect(
            host=host,
            user=user,
            password=password,
            database=database
        )
        self.cursor = self.connection.cursor()

    def insert_object(self, obj):
        table_name = type(obj).__name__
        attributes = vars(obj)
        columns = []
        values = []

        for column, value in attributes.items():
            if column != '__version__':
                columns.append(column)
                values.append(value)

        self.cursor.execute(f"SHOW TABLES LIKE '{table_name}'")
        table_exists = self.cursor.fetchone()

        if not table_exists:
            self.create_table(table_name, attributes)
        else:
            self.cursor.execute(f"DESCRIBE {table_name}")
            existing_columns = [column[0] for column in self.cursor.fetchall()]
            missing_columns = set(columns) - set(existing_columns)

            for column in missing_columns:
                self.add_column(table_name, column, attributes[column])

        placeholders = ', '.join(['%s'] * len(values))
        self.cursor.execute(f"INSERT INTO {table_name} ({', '.join(columns)}) VALUES ({placeholders})", values)
        self.connection.commit()

    def create_table(self, table_name, attributes):
        column_definitions = []
        for column, value in attributes.items():
            if column != '__version__':
                if isinstance(value, int):
                    column_definitions.append(f"{column} INT")
                elif isinstance(value, float):
                    column_definitions.append(f"{column} FLOAT")
                elif isinstance(value, str):
                    column_definitions.append(f"{column} VARCHAR(255)")
                elif isinstance(value, datetime):
                    column_definitions.append(f"{column} DATETIME")

        column_definitions.insert(0, "id INT AUTO_INCREMENT PRIMARY KEY")
        self.cursor.execute(f"CREATE TABLE {table_name} ({', '.join(column_definitions)})")

    def add_column(self, table_name, column, value):
        if isinstance(value, int):
            self.cursor.execute(f"ALTER TABLE {table_name} ADD COLUMN {column} INT")
        elif isinstance(value, float):
            self.cursor.execute(f"ALTER TABLE {table_name} ADD COLUMN {column} FLOAT")
        elif isinstance(value, str):
            self.cursor.execute(f"ALTER TABLE {table_name} ADD COLUMN {column} VARCHAR(255)")
        elif isinstance(value, datetime):
            self.cursor.execute(f"ALTER TABLE {table_name} ADD COLUMN {column} DATETIME")

    def query_object(self, cls, **kwargs):
        table_name = cls.__name__
        conditions = ' AND '.join([f"{column} = %s" for column in kwargs.keys()])
        values = list(kwargs.values())

        self.cursor.execute(f"SELECT * FROM {table_name} WHERE {conditions}", values)
        result = self.cursor.fetchone()

        if result:
            obj = cls()
            attributes = vars(obj)
            for i, column in enumerate(attributes.keys()):
                if column != '__version__':
                    setattr(obj, column, result[i])
            return obj
        else:
            return None

    def delete_object(self, cls, **kwargs):
        table_name = cls.__name__
        conditions = ' AND '.join([f"{column} = %s" for column in kwargs.keys()])
        values = list(kwargs.values())

        self.cursor.execute(f"DELETE FROM {table_name} WHERE {conditions}", values)
        self.connection.commit()

    def update_object(self, obj):
        table_name = type(obj).__name__
        attributes = vars(obj)
        columns = []
        values = []

        for column, value in attributes.items():
            if column != '__version__':
                columns.append(column)
                values.append(value)

        update_statements = ', '.join([f"{column} = %s" for column in columns])
        self.cursor.execute(f"UPDATE {table_name} SET {update_statements} WHERE id = %s", values + [obj.id])
        self.connection.commit()

    def close(self):
        self.cursor.close()
        self.connection.close()