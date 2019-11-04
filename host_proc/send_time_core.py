import serial
import time
import glob
import calendar
import datetime
import pytz
import os
import sys
import logging
from logging.handlers import TimedRotatingFileHandler

logger = logging.getLogger("send_time")
log_file = "logs/send_time.out"

def init_logger():
    formatter = logging.Formatter('%(asctime)s %(levelname)8s %(message)s',
                datefmt='%Y-%m-%d %H:%M:%S')

    logger.setLevel(logging.INFO)
    handler = TimedRotatingFileHandler(log_file,
                                        when="d",
                                        interval=1,
                                        backupCount=30)
    handler.setFormatter(formatter)
    screen_handler = logging.StreamHandler(stream=sys.stdout)
    screen_handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.addHandler(screen_handler)

    logger.info("Starting serial connection to Moteino")


def send_time(ser, no_data_time):
    try:
        data = ser.readline().decode("utf-8")
    except (OSError, serial.serialutil.SerialException):
        if time.time() - no_data_time < 10:
            raise Exception("Too much no data")
        no_data_time = time.time()
        logger.info("No data")
    if (len(data)>0):
        logger.info(data.strip())

    try:
        now = datetime.datetime.now(pytz.timezone('US/Eastern'))
        if data.index("SYN-REQ") >= 0:
            offset_hrs = int(now.utcoffset().total_seconds()/60/60)
            time_send = "TI"+str(int(now.timestamp())) \
                        +("+" if offset_hrs > 0 else "-") +str(abs(offset_hrs))
            ser.write(time_send.encode("utf-8"))
            logger.info("Sending "+time_send)
    except Exception as e:
        pass
    return no_data_time

