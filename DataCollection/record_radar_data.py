#Code to read serial data
import serial, serial.tools.list_ports #reading COM ports
import sys, os #allow keyboard interrupt to exit

from datetime import datetime
import time
import pandas as pd
def get_time():
    today = datetime.now()
    return today.strftime("%H:%M:%S")

def setDirectory():
    today = datetime.now()
    #dir = today.strftime("%d%m%y-%H%M")
    dir = today.strftime("%y-%m-%d_%H%M")
    return dir

def find_MCU_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if "USB" in p.description: #KitProg3 USB-UART for PSoc
            print(p.device + " is the port" )
            return p.device
    
    print("Unable to find valid port, COM0 returned")
    return 'COM0'

def read_data(mcu):
    numbers = ""
    while numbers == "":
        data = mcu.readline() #blocking, wait for next available data
        numbers = data.decode().strip() #decode data \n etc from serial data
    return numbers

def close_serial_port(mcu,msg):
    mcu.close()
    print(msg)

RESULTS_FOLDER = os.getcwd() + "\\Results\\"
SAVE_FOLDER = os.getcwd() + "\\Results\\" + setDirectory()
time_stamps = []
TD_results = []
PD_results = []

if __name__ == "__main__": #Only if file is run
    
    record_data = False

    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    PSoc6 = serial.Serial(
        port = PORT,
        baudrate=BAUDRATE,
        timeout=TIMEOUT,
        bytesize=8,
        parity= 'N',
        stopbits=1) #defined above
    
    START_TIME = time.time()
    PSoc6.reset_input_buffer()
    start = False

    while True:
        try:
            data = read_data(PSoc6)

            if (data == "9") and (start == False):
                start = True
                print("Data collection starting now")
                continue
            elif (data == "9") and (start == True):
                msg = "Data collection has ended"
                start = False
                close_serial_port(PSoc6,msg)
                break

            #time_stamps.append(round(time.time() - START_TIME,2))
            ts = get_time()
            time_stamps.append(ts)

            TD,PD = data.split(" ")
            TD_results.append(TD)
            PD_results.append(PD)

            print(ts," > ", data)
                
        except KeyboardInterrupt:
            msg = "Serial port " + str(PORT) + " closed by KeyboardInterrupt"
            close_serial_port(PSoc6,msg)
            break

        except:
            msg = str(PORT) + " closed by Other Error"
            close_serial_port(PSoc6,msg)
            break

    
print("Saving Data")
try:
    os.mkdir(SAVE_FOLDER)
except:
    print("Creating new results folder")
    os.mkdir(RESULTS_FOLDER)
    os.mkdir(SAVE_FOLDER)

df = pd.DataFrame(data = {"ts":time_stamps,"TD":TD_results, "PD":PD_results})
df.to_csv(SAVE_FOLDER + "\\results.csv")
