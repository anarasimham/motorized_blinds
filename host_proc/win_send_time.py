import send_time_core
import glob
import serial
import os
import logging

send_time_core.init_logger()

logger = logging.getLogger("send_time")

usb_path = glob.glob("/dev/ttyUSB*")
ser = serial.Serial("COM5",timeout=1,baudrate=115200)
no_data_time = 0

while True:
    mydir = "C:\\Users\\Ashish\\Documents\\Arduino\\automated_blinds\\do_send"
    files = os.listdir(mydir)

    if len(files) > 0:
        prefix = files[0][:4]
        if (prefix.startswith('B') or prefix.startswith('BC') or prefix.startswith('BO')):
            logger.info("Sending '"+prefix+"'")
            if prefix.startswith('BO') or prefix.startswith('BC'):
                ser.write(prefix[:2].encode("utf-8"))
            else:
                ser.write(prefix.encode("utf-8"))
            os.rename(mydir+"\\"+files[0], mydir+"\\"+"old_"+files[0])

    no_data_time = send_time_core.send_time(ser, no_data_time)
