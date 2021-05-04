from socket import *
import tkinter, math

CANV_SIZE = 500
HAPTIC_MAPPING = {'a':50, 'b':100, 'c':175, 'd':250, 'e':350, 'f':500, 'g':2, 'h':1000}
HAPTIC_ANGLES = [135,112.5,90,67.5,45]
_X = [(0,200), (200,400), (400,600), (600,800), (800,1000)]
ANGLE_WIDTH = 30
s=socket(AF_INET, SOCK_DGRAM)
s.bind(('',1234))

# Converts an angle-magnitude vector to component form
# output: x, y
def am_to_component(angle, mag):
    x = math.cos(math.radians(angle))*mag
    y = math.sin(math.radians(angle))*mag
    return x,y

def display_haptics():
    command=s.recvfrom(1024)
    command = command[0].decode()
    print(command)

    canv.delete('all')

    user_loc = [CANV_SIZE, CANV_SIZE*2 - 25]
    # Show user
    canv.create_rectangle(CANV_SIZE-10,CANV_SIZE*2 - 50, CANV_SIZE+10, CANV_SIZE*2 - 60)
    for i in range(5):
        c = command[i]
        dist = HAPTIC_MAPPING[c]

        """
        angle = HAPTIC_ANGLES[i]
        x1,y1 = am_to_component(angle+(ANGLE_WIDTH/2), dist)
        x2,y2 = am_to_component(angle-(ANGLE_WIDTH/2), dist)
        canv.create_polygon([user_loc[0], user_loc[1], x1+CANV_SIZE, y1, x2+CANV_SIZE, y2], outline='blue', fill='yellow', width=2)
        """

        poly = [_X[i][0], 1000, _X[i][1], CANV_SIZE*2 -dist]
        canv.create_rectangle(poly, outline='blue', fill='yellow', width=2)
        

    root.after(50,display_haptics)


root = tkinter.Tk()
canv = tkinter.Canvas(root, bg='black', height=CANV_SIZE*2, width=CANV_SIZE*2)
canv.pack()
root.after(50,display_haptics)
root.mainloop()
