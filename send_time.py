import serial
import time
import glob
import calendar
import datetime
import pytz
import os

usb_path = glob.glob("/dev/ttyUSB*")

print("Starting serial connection to Moteino")
ser = serial.Serial(usb_path[0],timeout=1,baudrate=115200)
no_data_time = 0
while True:
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
        #print(e)
        #print("SYN-REQ Not found")

