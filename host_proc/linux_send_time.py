import send_time_core
import serial

usb_path = glob.glob("/dev/ttyUSB*")

print("Starting serial connection to Moteino")
ser = serial.Serial(usb_path[0],timeout=1,baudrate=115200)
no_data_time = 0
while True:
    no_data_time = send_time_core.send_time(ser, no_data_time)

