import argparse
import commInterface as ci
import cv2
import flycapture2 as fc2
import numpy as np
import time
import os
import subprocess
import signal

BLUE = 0
GREEN = 1
WHITE = 2
YELLOW = 3
RED = 4
ORANGE = 5
OTHER = 6
COLORS = ['B', 'G', 'W', 'Y', 'R', 'O', '?']

RED_HUE = (1, 6)
ORANGE_HUE = (8, 10)
WHITE_YELLOW_HUE = (13, 29)
GREEN_HUE = (41, 60)
BLUE_HUE = (101, 140)

WHITE_SAT = (0, 139)
ORANGE_SAT = (140, 255)

HUES = [BLUE_HUE, GREEN_HUE, WHITE_YELLOW_HUE, WHITE_YELLOW_HUE, RED_HUE, ORANGE_HUE]
SATS = [WHITE_SAT, ORANGE_SAT]

STATE = [('R', 'top', 'top_full.txt', [6, 7, 8, 5, 2, 14, 17, 21, 24]),
             ('R', 'R1', 'top_right.txt', [25, 26]),
             ('R',  'R2', 'top_right_top.txt', [23, 20, 35, 32, 29]), # top_right + top_right_top
             ('R',  'R3', 'top_right_top.txt', [19, 18, 36, 39, 42]), # top_right + top_right_top
             ('F', None, None, None),
             ('F', 'F1', 'top_left_top.txt', [10, 11, 53, 50, 47]), # top_left + top_left_top
             ('F', 'F2', 'top_left_top_2.txt', [12, 9, 28, 27]), # top_left + top_left_top without center
             ('F', 'F3', 'top_left.txt', [16, 15]),
             ('U2', None, None, None),
             ('D2', 'U1', 'top_top.txt', [1, 0, 3]),
             ('R', 'R4', 'top_right_2.txt', [51]),       # bottom piece of top right
             ('R', 'R5', 'top_right_1.txt', [52]),       # top piece of top_right
             ('R', 'R6', 'top_right_top_2.txt', [30, 33]),     # top_top_right without center
             ('R', 'R7', 'top_right.txt', [46, 45]),  
             ('F', 'F4', 'top_left_2.txt', [44]),    
             ('F', 'F5', 'top_left.txt', [37, 38]),        # bottom piece of top left
             ('F', 'F6', 'top_top_left_center.txt', [34]),        # top_top_right center center piece .. X ..
             ('F', 'F7', 'top_left_1.txt', [43]),        # top pierce of top_left 
             ('U2', None, None, None),
             ('D2', None, None, None),
             ('B2', None, None, None),
             ('R2', None, None, None),
             ('R2', 'LAST', 'top_left_right.txt', [41, 48]),  # center points for left and right
             ('B2', None, None, None)]

# TOP_STATE = [('R', 'top', 'top_full.txt', [6, 7, 8, 5, 2, 14, 17, 21, 24]),
#              ('R', 'R1', 'top_right.txt', [25, 26]),
#              ('R',  'R2', 'top_right.txt', [23, 20]),
#              ('R',  'R3', 'top_right.txt', [19, 18]),
#              ('F', None, None, None),
#              ('F', 'F1', 'top_left.txt', [10, 11]),
#              ('F', 'F2', 'top_left.txt', [12, 9]),
#              ('F', 'F3', 'top_left.txt', [16, 15]),
#              ('U2', None, None, None),
#              ('U2', 'U1', 'top_top.txt', [3, 0, 1])]

# BOTTOM_STATE = [('L', 'bottom', 'bottom_full.txt', [35, 34, 33, 30, 27, 42, 43, 53, 52]),
#                 ('L', 'L1', 'bottom_right.txt', [47, 50]),
#                 ('L', 'L2', 'bottom_right.txt', [45, 46]),
#                 ('L', 'L3', 'bottom_right.txt', [51, 48]),
#                 ('B', None, None, None),
#                 ('B','B1', 'bottom_left.txt', [44, 41]),
#                 ('B','B2', 'bottom_left.txt', [38, 37]),
#                 ('B','B3', 'bottom_left.txt', [36, 39]),
#                 ('D2', None, None, None),
#                 ('D2', 'D1', 'bottom_bottom.txt', [28, 29, 32])]

# Define a set of points for each file to use and the index that it is associated with


def syncExecuteMoves(HerbertSerialConnection, move_sequence):
    fullPacket = ci.generateFullCommandPacket("ExecuteMoves " + move_sequence, 0)
    response = ci.sendCommand(HerbertSerialConnection, fullPacket)
    while(True):
        fullPacket = ci.generateFullCommandPacket("IsIdle", 0)
        response = ci.sendCommand(HerbertSerialConnection, fullPacket)
        if(int(response.split()[4]) == 1):
            break

def abortSolutionProcess(HerbertSerialConnection):
    fullPacket = ci.generateFullCommandPacket("Abort", 0)
    ci.sendCommand(HerbertSerialConnection, fullPacket)

def captureCubeState(HerbertSerialConnection, captureTop, camera_id):
    # Need to set the camera brightness, exposure and etc
    camera_context = fc2.Context()
    camera_context.connect(*camera_context.get_camera_from_index(camera_id))
    camera_context.set_video_mode_and_frame_rate(fc2.VIDEOMODE_1280x960Y8,
            fc2.FRAMERATE_7_5)  

    state = STATE
    camera_context.start_capture()

    for st in state:
        if(st[1] != None):
            image = fc2.Image()
            camera_context.retrieve_buffer(image)
            camera_context.retrieve_buffer(image)
            image = fc2.convert(fc2.PIXEL_FORMAT_BGR, image)
            image.save(st[1] + '.bmp', fc2.BMP) # The filename is not finalized
        syncExecuteMoves(HerbertSerialConnection, st[0])
        
    camera_context.stop_capture()
    camera_context.disconnect()

# Old code for categorizing code
def classify_pixel(pixel):
    hue = pixel[0]
    sat = pixel[1]
    val = pixel[2]

    #Brightness 1.184 and ev -0.585
    if(hue == 0 and sat == 0 and val == 0): # Black
        return OTHER
    elif(hue == 0 and val == 255): # One shade of white
        return WHITE
    elif(hue < 7):
        return RED
    elif(hue > 7 and hue <= 10):
        return ORANGE
    elif(hue > 12 and hue < 30):
        if(hue > 15 and sat < 140):
            return WHITE
        else:
            return YELLOW
    elif(hue > 40 and hue <= 60):
        return GREEN
    elif(hue > 100 and hue <= 140):
        return BLUE
    else:
        return OTHER

def categorizeCubeState(optimized):
    facelets = np.chararray(54)

    facelets[4] = COLORS[WHITE]
    facelets[13] = COLORS[RED]
    facelets[22] = COLORS[BLUE]
    facelets[31] = COLORS[YELLOW]
    facelets[40] = COLORS[ORANGE]
    facelets[49] = COLORS[GREEN]

    for state in STATE: #TOP_STATE+BOTTOM_STATE:
        if(state[1] != None):
            bgr_img = cv2.imread(state[1] + '.bmp')

            with open(state[2]) as f:
                points = [map(int, line.split()) for line in f]

            polygons = []
            # Loop through the image and sample the selected points in HSV
            for i in xrange(0, len(points), 4):
                polygons.append([points[i], points[i + 1], points[i + 2], points[i + 3]])

            # Convert the BGR image into HSV
            hsv_img = cv2.cvtColor(bgr_img, cv2.COLOR_BGR2HSV)

            facelet_index = 0
            for polygon in polygons:
                facelet_mask = np.zeros(hsv_img.shape[:2], np.uint8)
                cv2.fillPoly(facelet_mask, [np.array(polygon)], (255, 255,255))
                masked_hsv_img = cv2.bitwise_and(hsv_img, hsv_img, mask = facelet_mask)

                if optimized:
                    hue_hist = cv2.calcHist([masked_hsv_img], [0], None, [180], [1, 180])
                    sat_hist = cv2.calcHist([masked_hsv_img], [1], None, [256], [1, 256])
                    hue_freq = [0, 0, 0, 0, 0, 0]
                    sat_freq = [0, 0]

                    for hue_range in HUES:
                        for hue in range(hue_range[0], hue_range[1] + 1):
                            hue_freq[HUES.index(hue_range)] = hue_freq[HUES.index(hue_range)] + hue_hist[hue - 1]

                    if(max(hue_freq) == hue_freq[HUES.index(WHITE_YELLOW_HUE)]):
                        for sat_range in SATS:
                            for sat in range(sat_range[0], sat_range[1] + 1):
                                sat_freq[SATS.index(sat_range)] = sat_freq[SATS.index(sat_range)] + sat_hist[sat - 1]

                        if(sat_freq[0] > sat_freq[1]):
                            color = COLORS[WHITE]
                        else:
                            color = COLORS[YELLOW]
                    else:
                        color = COLORS[hue_freq.index(max(hue_freq))]
                else: 
                    # # Calculate a bounding rectangle
                    ret,thresh = cv2.threshold(facelet_mask,127,255,cv2.THRESH_BINARY)
                    contours = cv2.findContours(thresh, 1, 2)
                    x_start,y_start,w,h = cv2.boundingRect(contours[0])

                    # Categorize the colors
                    count = [0, 0, 0, 0, 0, 0, 0]
                    for x in range(x_start, x_start+w):
                        for y in range(y_start, y_start+h):
                            count[classify_pixel(masked_hsv_img[y][x])]+=1
                    color = COLORS[count.index(max(count[:6]))]

                facelets[state[3][facelet_index]] = color
                facelet_index = facelet_index + 1

    cube_state = formatCubeState(facelets)
    print cube_state
    return cube_state

def formatCubeState(facelets):
    cubeState = ["", "", "", "", "", ""]
    faces = ['U', 'F', 'R', 'D', 'B', 'L']
    for i in range(0, 54):
        if(i % 9 == 0):
            cubeState[i / 9] = cubeState[i / 9] + faces[i / 9] + ':'
        cubeState[i / 9] = cubeState[i / 9] + facelets[i]
    return cubeState

def getCubeSolution(rep, kcube_path, t_thresh=2):
    """
    Gets the solution sequence for the supplied Rubik's representation.

    :rep: The cube representation (see function 'get_cube_rep' above).
    :kcube_path: The path to the KCube executable.
    :sol_threshold: The threshold indicating the minimum number of moves.
    """

    # Change into the directory where KCube lives
    os.chdir(kcube_path)

    proc_time = None

    # Invoke the KCube application with the supplied cube representation
    process = subprocess.Popen(["KCUBE", rep[0], rep[1], rep[2], rep[3], rep[4], rep[5]], stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
    starttime = time.time()

    # Poll the ongoing process for new output until finished
    # Solution_found = False
    sol_sequence = None

    while (time.time() - starttime) < t_thresh  or sol_sequence == None:
        nextline = process.stdout.readline()

        if nextline == '' and process.poll() != None:
            break

        for x in range(25):
            if "(" + str(25 - x) + ")" in nextline and not "threshold" in nextline:
                sol_sequence = nextline

    endtime = time.time()
    proc_time = endtime - starttime

    # if the sol_sequence is not empty, replace the period and remove the last 4 character assum 
    if(sol_sequence != None):
        sol_sequence = sol_sequence.replace(". ", "").replace("'", "b").replace("'", "").split("(")[:-1][0]

    # Kill the Kcube process and return

    return (sol_sequence, proc_time)

if __name__ == '__main__':
    print "Finished importing packages..."
    
    start_time = time.time()
    # Create the CLI options
    parser = argparse.ArgumentParser(description='')
    parser.add_argument("--id", required=True, type=int, help="The device ID for top Camera.")
    parser.add_argument("--disableOptimized", "-do", action='store_false', help="Disable optimization.")
    parser.add_argument("--sequenceMode", "-fast", action='store_true', help="Disable optimization.")
    parser.add_argument("--debug", "-d", action='store_true', help="Enable debugging.")
    parser.add_argument("--t_thresh", "-thresh", default=2, type=int, help="Amount of time to look for optimal solution.")
    parser.add_argument("--port", "-p", default="COM9", help="The COM port which the FPGA is plugged into.")

    args = parser.parse_args()
    debug = args.debug

    HerbertSerialConnection = ci.HerbertSerial(args.port)
    try:
        # Captures the cube state
        captureCubeState(HerbertSerialConnection, True, args.id)

        # Get the solution sequence by invoking Kociemba's algorithm on the cube
        # representation
        kcube_path = "kcube"
        (sol_seq, proc_time) = getCubeSolution(categorizeCubeState(args.disableOptimized), kcube_path, args.t_thresh)
        print sol_seq
        if sol_seq != None:
            # If sequence mode is true then it will send execute command one at a time
            if args.sequenceMode: 
                syncExecuteMoves(HerbertSerialConnection, sol_seq)
            else:
                for move in sol_seq.split():
                    if move != "":
                        syncExecuteMoves(HerbertSerialConnection, move)

        print time.time() - start_time
    except KeyboardInterrupt:
        print "Aborting Gracefully!"
    finally:
        abortSolutionProcess(HerbertSerialConnection)
        HerbertSerialConnection.close()

