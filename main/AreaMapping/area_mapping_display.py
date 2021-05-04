import socket, math, tkinter, time, serial, datetime, sys
from multiprocessing import Process, Queue, Manager


##### Global variables #####
#####    constants     #####

# Lidar and gyroscope indices
L1 = int(0)
L2 = int(1)
L3 = int(2)
GY = int(3)
GX = int(4)
GZ = int(5)
AX = int(6)
AY = int(7)
AZ = int(8)

L1_ANGLE = int(345)
L2_ANGLE = int(0)
L3_ANGLE = int(15)

MAX_DIST = 50 # Maximum distance that can be read from the LiDAR sensors

EMPTY = -2 # No stored value for angle
NULL = -1 # INF value for angle/ out of range

DPI = int(6) # Degrees per index
SLICES = int(60)

DEBUG_X_OFFSET = 5
DEBUG_Y_OFFSET = 20
DEBUG = True

CANV_SIZE = 250

HOST = '127.0.0.1'
PORT = 2005

SERIAL_PORT = 'COM18'
BAUD_RATE = 115200

#####     variables     #####

area = [(i*DPI,EMPTY) for i in range(SLICES)]

sensors = None

min_dist = 0
max_dist = 1

serial_queue = None
serial_connection = None


def read_sensors():
    #data = conn.recv(2048)
    data = sock.recvfrom(1024)
    #print(data[0])
    data = data[0].decode().split("\t")
    for i in range(len(data)):
        try:
            data[i] = float(data[i])
        except ValueError:
            data[i] = NULL
    s = ""
    for value in data:
        s += str(value) + " "
    print(s)
    return data

# Converts a component-form vector to angle-magnitude
# output: angle, magnitude
def component_to_am(x, y):
    mag = math.sqrt((x*x)+(y*y))
    angle = math.degrees(math.atan2(y,x))+180
    return angle, mag

# Converts an angle-magnitude vector to component form
# output: x, y
def am_to_component(angle, mag):
    x = math.cos(math.radians(angle))*mag
    y = math.sin(math.radians(angle))*mag
    return x,y

# Returns the slope and y-intercept of a line between two points
# output: m, b
def slope(x1, y1, x2, y2):
    m = (y2-y1)/(x2-x1)
    b = y2 - (m*x2)
    return m,b

# Returns the intersection of two lines
# output: x, y
def intersection(m1, b1, m2, b2):
    x = (b2 - b1)/(m1 - m2)
    y = (m1 * ((b2 - b1)/(m1 - m2))) + b1
    return x,y

# Returns the coeficient to normalize the range
# [0, old_max] to the range [0, new_max]
# output: ⌊old_max/new_max⌉
def normalize(old_max, new_max):
    return int(round(old_max//new_max))

# Normalizes n from the range [0, old_max] to [0, new_max]
# output: n'
def min_max_norm(n,old_max, new_max):
    return (n/old_max) * (new_max)

# Returns n's nearest mutliple of k.
# k defaults to DPI, and only the quotient of
# the multiple is returned, not multiple itself.
def nearest(n, k=DPI):
    return int(round(float(n/k)))

# Inputs the distance values in sensors into
# the mapped area
# output: void
def update_distance(sensors, a1, a2, a3, debug_strings):
    global L1_ANGLE, L2_ANGLE, L3_ANGLE, DPI, max_dist, area

    start_time = datetime.datetime.now().microsecond

    # Checks if the maximum distance in the mapped area has
    # changed.
    if(sensors[L1] > max_dist):
        max_dist = sensors[L1]
    elif(sensors[L2] > max_dist):
        max_dist = sensors[L2]
    elif(sensors[L3] > max_dist):
        max_dist = sensors[L3]
    reset_max = False
    if(area[nearest(a1)%SLICES][1] == max_dist):
        reset_max = True
    elif(area[nearest(a2)%SLICES][1] == max_dist):
        reset_max = True
    elif(area[nearest(a3)%SLICES][1] == max_dist):
        reset_max = True

    # Inserts the new distance values into the mapped area
    area[nearest(a1)%SLICES] = (a1,sensors[L1])
    area[nearest(a2)%SLICES] = (a2,sensors[L2])
    area[nearest(a3)%SLICES] = (a3,sensors[L3])

    # Finds the current maximum distance in the mapped area
    # if the old max was overwritten with one of the newly
    # input distance values
    if(reset_max):
        for i in range(len(area)):
            if(area[i][1] > max_dist):
                max_dist = area[i][1]

    if(DEBUG):
        debug_strings.append("update_distance: {}".format(datetime.datetime.now().microsecond - start_time))

# Returns the distance/approximated distance at an angle +-
# tolerance*DPI degrees from the origin of the mapped area.
# if the angle is divisible by DPI, an exact value will be
# returned, else a best approximation is computed based on
# the nearest distances within the tolerance range.
# output: distance[angle +- tolerance]
def query_distance(angle, tolerance):
    if(angle % DPI == 0):
       return area[nearest(angle)%SLICES][1]
    else:
        index = nearest(angle)%SLICES
        return area[index][1]
        
        m = int(math.ceil(float(angle/DPI))*DPI) % SLICES
        n = int(math.floor(float(angle/DPI))*DPI) % SLICES

        if(area[m][1] < 0):
            for i in range(m, m+tolerance+1):
                if(area[i%SLICES][1] >=0):
                    m = i%SLICES
                    break
                else:
                    m = NULL
        if(area[n][1] < 0):
            for i in range(m, m-tolerance-1, -1):
                if(area[i%SLICES][1] >=0):
                    n = i%SLICES
                    break
                else:
                    n = NULL
        if(m == NULL and n == NULL):
            return 0
        elif(m == NULL or n == NULL):
            m = max(m, n)
            return area[m][1]
        else:
            m_val= area[nearest(m*DPI)%SLICES][1]
            n_val = area[nearest(n*DPI)%SLICES][1]
            return ((((m-n)/(angle-n))*m_val) + ((1-((m-n)/(angle-n)))*n_val))
            
def update_location(dx, dy, dz, debug_strings):
    global EMPY, NULL, DPI, area, sensors

    start_time = datetime.datetime.now().microsecond
    
    # Exit if there is no movement
    if(dx==0 and dz==0):
        if(DEBUG):
            debug_strings.append("update_location: {}".format(datetime.datetime.now().microsecond - start_time))
        return

    # Create new area array to replace the old one
    _area = [(i*DPI, EMPTY) for i in range(SLICES)]
    for i in range(len(area)):
        if(area[i][1] == NULL or area[i][1] == EMPTY):
            continue
        angle = area[i][0] + 180 % 360
        dist = area[i][1]
        # Previous coordinates of this measured point
        zi,xi = am_to_component(angle, dist)
        # Adjust the coordinates based on user movement
        zi += dz
        xi += dx
        # Convert back to angle, distance and place in new map
        angle, dist = component_to_am(zi, xi)
        _area[nearest(angle)%SLICES] = (angle % 360, dist)

    # Replaces the previous map with the updated one
    area = _area

    if(DEBUG):
        debug_strings.append("update_location: {}".format(datetime.datetime.now().microsecond - start_time))

def draw_area(canv_coef, x_angle, a1, a2, a3, debug_strings):
    global area, EMPTY, NULL, DPI, L1_ANGLE, L2_ANGLE, L3_ANGLE

    start_time = datetime.datetime.now().microsecond
    
    # Clears the GUI
    canv.delete('all')
    
    # Shows the user's location
    canv.create_rectangle(CANV_SIZE-1,CANV_SIZE-1,CANV_SIZE+1,CANV_SIZE+1,fill='yellow',outline='yellow')

    # Draw the mapped area
    for i in range(len(area)):
        if(area[i][1] == EMPTY or area[i][1] == NULL):
            continue
        p1 = am_to_component(*area[i])
        p2 = None
        for j in range(i+1, i+11):
            if(area[j%SLICES][1] == EMPTY or area[j%SLICES][1] == NULL):
                continue
            p2 = am_to_component(*area[j%SLICES])
            break
        if(p2 is not None):
            canv.create_line(CANV_SIZE+(p1[0]*canv_coef), CANV_SIZE+(p1[1]*canv_coef), CANV_SIZE+(p2[0]*canv_coef), CANV_SIZE+(p2[1]*canv_coef), fill='orange')
            canv.create_line(CANV_SIZE-1, CANV_SIZE-1, CANV_SIZE+(p1[0]*canv_coef), CANV_SIZE+(p1[1]*canv_coef), fill='gray10')
            
    # Draws lidar rays
    x,y = am_to_component((L1_ANGLE+x_angle)%360, area[nearest(L1_ANGLE+x_angle)%SLICES][1])
    canv.create_line(CANV_SIZE-1, CANV_SIZE-1, CANV_SIZE+(x*canv_coef), CANV_SIZE+(y*canv_coef), fill='white')
    x,y = am_to_component((L2_ANGLE+x_angle)%360, area[nearest(L2_ANGLE+x_angle)%SLICES][1])
    canv.create_line(CANV_SIZE-1, CANV_SIZE-1, CANV_SIZE+(x*canv_coef), CANV_SIZE+(y*canv_coef), fill='blue')
    x,y = am_to_component((L3_ANGLE+x_angle)%360, area[nearest(L3_ANGLE+x_angle)%SLICES][1])
    canv.create_line(CANV_SIZE-1, CANV_SIZE-1, CANV_SIZE+(x*canv_coef), CANV_SIZE+(y*canv_coef), fill='green')

    if(DEBUG):
        debug_strings.append("draw_area: {}".format(datetime.datetime.now().microsecond - start_time))
    
# Calculates true distance to the measured object,
# not the distance measured from an indirect angle.
def normalize_distances(sensors, rel_angle, debug_strings):
    global L1, L2, L3, MAX_DIST, CANV_SIZE
    # Only necessary when the camera can be pointed up/down
    """ 
    sensors[L1] = rel_angle*sensors[L1]
    sensors[L1] = min_max_norm(sensors[L1], MAX_DIST, CANV_SIZE)
    sensors[L2] = rel_angle*sensors[L2]
    sensors[L2] = min_max_norm(sensors[L2], MAX_DIST, CANV_SIZE)
    sensors[L3] = rel_angle*sensors[L3]
    sensors[L3] = min_max_norm(sensors[L3], MAX_DIST, CANV_SIZE)
    """

def control_haptics(x_angle, serial_queue, debug_strings):
    # To be improved.
    haptic_1 = query_distance((x_angle - 60) % 360, 5)
    haptic_2 = query_distance((x_angle - 30) % 360, 5)
    haptic_3 = query_distance((x_angle) % 360, 5)
    haptic_4 = query_distance((x_angle + 30) % 360, 5)
    haptic_5 = query_distance((x_angle + 60) % 360, 5)
    motors = [haptic_1, haptic_2, haptic_3, haptic_4, haptic_5]

    # range = [-2,~30)
    intensity = ['a','b','c','d','e','f','g','h']
    #intensity_values = [0, 125, 145, 160, 180, 200, 220, 255]
    intensity_mappings = [(25,50),(18,25),(11,18),(5,11),(4,6),(2,4),(1,3),(0,1)]

    # Create the command to send to the haptic device
    command = ""
    for i in range(len(motors)):
        if(motors[i] <= 0):
            command = command + intensity[0]
        else:
            if(motors[i] >= 50):
                command = command + intensity[-1]
                continue
            else:
                for j in range(len(intensity_mappings)):
                    if(motors[i] >= intensity_mappings[j][0] and motors[i] < intensity_mappings[j][1]):
                        command = command + intensity[j]
                        break
                    
    if(DEBUG):
        haptic_debug = ""
        for g in motors:
            try:
                haptic_debug = haptic_debug + str(round(g,1)) +',\t'
            except:
                "" # Not worth the effort here
        debug_strings.append(haptic_debug)
        debug_strings.append(command)

    # output command to serial 
    serial_print(command)

def map_area():
    global L1, L2, L3, GX, GY, GZ, AX, AY, AZ, EMPTY, \
           NULL, DPI, MAX_DIST, CANV_SIZE, L1_ANGLE, \
           L2_ANGLE, L3_ANGLE, canv, area, max_dist, \
           sensors, serial_queue

    start_time = datetime.datetime.now().microsecond

    debug_strings = []

    # Gets the current states from the sensors
    ls = sensors
    sensors = read_sensors()
    for i in range(3):
        if(sensors[i] > MAX_DIST):
            sensors[i] = ls[i]
        elif(sensors[i] < -MAX_DIST):
            sensors[i] = ls[i]
    sensors[GX] = math.degrees(sensors[GX])
    sensors[GY] = math.degrees(sensors[GY])
    sensors[GZ] = math.degrees(sensors[GZ])
    
    # Canvas mulitplier
    canv_coef = normalize(CANV_SIZE, max_dist)
    
    # Translate the user with the mapped area
    #update_location(sensors[AX], sensors[AY], sensors[AZ], debug_strings)

    y_angle = sensors[GY] # vertical angle of the user relative to the horizon
    x_angle = sensors[GX] # forward angle of the user relative to starting rotation
    rel_angle = math.cos(math.radians(y_angle))

    # Indices for each of the distance sensors to be
    # inserted into the area list
    L1_index = (L1_ANGLE + x_angle) % 360
    L2_index = (L2_ANGLE + x_angle) % 360
    L3_index = (L3_ANGLE + x_angle) % 360

    # Normalize distances
    normalize_distances(sensors, rel_angle, debug_strings)
    
    # Update distance values from sensors
    update_distance(sensors, L1_index, L2_index, L3_index, debug_strings)
    
    # Draws the mapped area to the screen
    draw_area(canv_coef, x_angle, L1_index, L2_index, L3_index, debug_strings)

    # Controls haptic device output
    #control_haptics(x_angle, serial_queue, debug_strings)

    if(DEBUG):
        debug_strings.append("map_area: {}".format(datetime.datetime.now().microsecond - start_time))
        for i in range(len(debug_strings)):
            canv.create_text(DEBUG_X_OFFSET,DEBUG_Y_OFFSET*(i+1),anchor='sw',text=debug_strings[i],fill='white')
    
    # Schedules the next update and GUI refresh
    #for point in area:
    #    print("{}, {}:".format(point[0],point[1]),end="")
    #print("\n")
    root.after(1,map_area)

#########  Subprocess for serial communication #########

def serial_communication(queue):
    # Establish serial connection
    #serial_connection = connect(SERIAL_PORT, BAUD_RATE)
    while(True):
        if(not queue.Empty):
            ""
            #serial_print(queue.get())

def serial_print(command):
    serial_connection.write(command.encode('ascii'))

# connect to serial
def connect(port,baud):
    try:
        return serial.Serial(port=port,baudrate=baud,timeout=5)
    except:
        print('Could not connect on serial port {}'.format(port))
        exit(1)


if( __name__ == '__main__'):

    # Spawn the process for serial communication
    #process_manager = Manager()
    #serial_queue = Queue()
    #serial_process = Process(target=serial_communication, args=(serial_queue,))
    #serial_process.start()
    #serial_connection = connect(SERIAL_PORT, BAUD_RATE)

    # Initialize the empty mapped area
    area = [(i*DPI,EMPTY) for i in range(SLICES)]
    sensors = None
    min_dist = 0
    max_dist = 1
    
    # Socket connection to sensors
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('',2005))
    #s.listen()
    #conn, addr = s.accept()

    # UI 
    root = tkinter.Tk()
    canv = tkinter.Canvas(root, bg='black', height=CANV_SIZE*2, width=CANV_SIZE*2)
    canv.pack()
    root.after(50,map_area)
    root.mainloop()

    serial_process.join()



