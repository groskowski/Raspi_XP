import time
import logging
from logging_handler import LoggingHandler
from database import get_db_connection
from spi_comms import SPIComs
from queue_manager import check_requests

logger = logging.getLogger()
logger.setLevel(logging.INFO)
handler = LoggingHandler(get_db_connection())
logger.addHandler(handler)

while True:
    try:
        coms = SPIComs()
        check_requests(coms)
    except Exception as e:
        logging.error(f"Request checking service crashed: {str(e)}")