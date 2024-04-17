import logging
from database import DatabaseManager

class LoggingHandler(logging.Handler):
    def __init__(self, manager: DatabaseManager):
        """
        Initialize the LoggingHandler with the given DatabaseManager.

        :param manager: The DatabaseManager instance for logging to the database.
        """
        logging.Handler.__init__(self)
        self.db = manager

    def emit(self, record):
        """
        Emit a log record.

        :param record: The log record to be emitted.
        """
        cursor = None
        try:
            msg = self.format(record)
            query = "INSERT INTO log (message, type) VALUES (%s, %s)"
            self.db.execute_query(query, (msg, record.levelname.lower()))
        except Exception as e:
            logging.error(f"Error emitting log record: {str(e)}")
            raise
        finally:
            if cursor:
                cursor.close()

    def close(self):
        """
        Close the LoggingHandler and the associated DatabaseManager.
        """
        try:
            self.db.close()
            logging.Handler.close(self)
        except Exception as e:
            logging.error(f"Error closing LoggingHandler: {str(e)}")
            raise