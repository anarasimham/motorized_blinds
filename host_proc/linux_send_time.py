import send_time_core
import serial
import glob
import logging
import traceback

send_time_core.init_logger()
logger = logging.getLogger("send_time")

logger.info("Starting serial connection to Moteino")

usb_path = glob.glob("/dev/ttyUSB*")

try:
    ser = serial.Serial(usb_path[0],timeout=1,baudrate=115200, exclusive=True)
    no_data_time = 0
    while True:
        no_data_time = send_time_core.send_time(ser, no_data_time)
except:
    logger.error(traceback.format_exc())
finally:
    ser.close()

