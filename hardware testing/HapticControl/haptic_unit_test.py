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

# log the data in a file
def log_data(data, value_in, value_out):
    data['TESTS'].append((value_in, value_out))

# write to the file
def save_data_log(data, path):
    timestamp = str(datetime.datetime.now()).replace(':','').replace('.','')
    log_file = open(LOG_FILE_PATH.format(timestamp), 'w')
    log_file.write(json.dumps(data))
    log_file.close()

# test the values we want to test, this will generally be intensity vs distance
def test_value(serial_connection, data, value):
    serial_connection.write(str(value).encode('ascii'))
    value_out = serial_connection.readline().decode('ascii').strip()
    log_data(data, value, value_out)
    return value

# statics
SERIAL_PORT = 'COM6'
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
data['MIN_DISTANCE'] = MIN_DISTANCE
data['MAX_DISTANCE'] = MAX_DISTANCE
data['MIN_MOTOR_SPEED'] = MIN_MOTOR_SPEED
data['MAX_MOTOR_SPEED'] = MAX_MOTOR_SPEED
data['TESTS'] = []

for value in range(MIN_DISTANCE - 5, MAX_DISTANCE + 5):
    print('[{}'.format(value),end=',')
    print('{}]'.format(test_value(serial_connection, data, value)))

disconnect(serial_connection)
save_data_log(data, LOG_FILE_PATH)
