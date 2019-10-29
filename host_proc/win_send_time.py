import serial
import time
import glob
import calendar
import datetime
import pytz
import os

usb_path = glob.glob("/dev/ttyUSB*")

ser = serial.Serial("COM5",timeout=1,baudrate=115200)
timer = time.time()
state = "BC"

while True:
    mydir = "C:\\Users\\Ashish\\Documents\\Arduino\\automated_blinds\\do_send"
    files = os.listdir(mydir)
    """if time.time() - timer > 12:
        if state is "BC":
            ser.write(state.encode("utf-8"))
            state = "BO"
        else:
            ser.write(state.encode("utf-8"))
            state = "BC"
        timer = time.time()"""
    if len(files) > 0:
        prefix = files[0][:4]
        if (prefix.startswith('B') or prefix.startswith('BC') or prefix.startswith('BO')):
            print("Sending '"+prefix+"'")
            if prefix.startswith('BO') or prefix.startswith('BC'):
                ser.write(prefix[:2].encode("utf-8"))
            else:
                ser.write(prefix.encode("utf-8"))
            os.rename(mydir+"\\"+files[0], mydir+"\\"+"old_"+files[0])

    data = ser.readline().decode("utf-8")
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
