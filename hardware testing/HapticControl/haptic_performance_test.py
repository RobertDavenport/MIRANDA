# imports
import serial
import time
import datetime
import json

# connect to serial
def connect(port,baud):
    try:
        return serial.Serial(port=port,baudrate=baud,timeout=5)
    except:
        print('Could not connect on serial port {}'.format(port))
        exit(1)

# disconnect from serial
def disconnect(serial_connection):
    serial_connection.close()

# write to the log file
def save_data_log(data, path):
    timestamp = str(datetime.datetime.now()).replace(':','').replace('.','')
    log_file = open(LOG_FILE_PATH.format(timestamp), 'w')
    log_file.write(json.dumps(data))
    log_file.close()

# take the average of the test data
def avg(data):
    total = 0
    for value in data['TESTS']:
        total += value
    average = total/len(data['TESTS'])
    print("Average", average)

# get the extra data (worst/best) of the tests
def extr(data):
    worst = data['TESTS'][0]
    best = data['TESTS'][0]
    for value in data['TESTS']:
        value = value
        if value > worst:
            worst = value
        if value < best:
            best = value
    print("Best", best)
    print("Worst", worst)

# statics
SERIAL_PORT = 'COM8'
BAUD_RATE = 115200
LOG_FILE_PATH = 'LOG{}.txt'

# Haptic controller values and I/O constraints
MAX_DISTANCE = 1023
MIN_DISTANCE = 0
MAX_MOTOR_SPEED = 255
MIN_MOTOR_SPEED = 0

##################### TESTING #####################
print('Connecting on COM port: {}...'.format(SERIAL_PORT))
serial_connection = connect(SERIAL_PORT, BAUD_RATE)
print('Connected.')

data = {}
data['TESTS'] = []

try:
    while True:
        try:
            value_out = int(serial_connection.readline().decode('ascii').strip())
            data['TESTS'].append(value_out)
        except ValueError:
            continue
except KeyboardInterrupt:
    disconnect(serial_connection)
    avg(data)
    extr(data)
    save_data_log(data, LOG_FILE_PATH)
