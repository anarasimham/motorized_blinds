import serial
import time
import glob
import calendar
import datetime
import pytz
import os


def send_time(ser, no_data_time):
    try:
        data = ser.readline().decode("utf-8")
    except (OSError, serial.serialutil.SerialException):
        if time.time() - no_data_time < 10:
            raise Exception("Too much no data")
        no_data_time = time.time()
        print("No data")

    if(len(data)>0):
        print("received data from moteino: "+data, end="")
    try:
        if data.index("SYN-REQ") >= 0:
            now = datetime.datetime.now(pytz.timezone('US/Eastern'))
            offset_hrs = int(now.utcoffset().total_seconds()/60/60)
            time_send = "TI"+str(int(now.timestamp())) \
                        +("+" if offset_hrs > 0 else "-") +str(abs(offset_hrs))
            ser.write(time_send.encode("utf-8"))
            print("Sending "+time_send)
    except Exception as e:
        pass
    return no_data_time

