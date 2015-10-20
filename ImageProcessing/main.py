import argparse
import os
import signal
import time
import subprocess
import commInterface

def get_cube_rep(id1, id2, debug):
    """
    Gets the Rubik's cube representation by processing the frames captured by the
    connected cameras.

    This function executes the C++ image processing application and returns the
    output from that application. The C++ image processing application returns a
    representation of the Rubik's cube captured by the connected cameras. The format
    of the output is as follows:

    L:L1L2L3L4L5L6L7L8L9 F:F1F2F3F4F5F6F7F8F9 U:U1U2U3U4U5U6U7U8U9 \
    R:R1R2R3R4R5R6R7R8R9 B:B1B2B3B4B5B6B7B8B9 D:D1D2D3D4D5D6D7D8D9

    L: Left Face -- L1-L9 representing the color chars for the facelets respectively.
    F: Front Face -- F1-F9 representing the color chars for the facelets respectively.
    U: Upper Face -- U1-U9 representing the color chars for the facelets respectively.
    R: Right Face -- R1-R9 representing the color chars for the facelets respectively.
    B: Back Face -- B1-B9 representing the color chars for the facelets respectively.
    D: Downward Face -- D1-D9 representing the color chars for the facelets respectively.

    :id1: The device ID for Camera 1.
    :id2: The device ID for Camera 2.
    """

    if debug:
        cmd = "./main {0} {1} -d".format(id1, id2)
    else:
        cmd = "./main {0} {1}".format(id1, id2)

    process = subprocess.Popen([cmd], shell=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)

    output = process.stdout.readline()

    return output.strip().split()

def get_cube_sol(rep, kcube_path, sol_threshold):
    """
    Gets the solution sequence for the supplied Rubik's representation.

    :rep: The cube representation (see function 'get_cube_rep' above).
    :kcube_path: The path to the KCube executable.
    :sol_threshold: The threshold indicating the minimum number of moves.
    """

    # change into the directory where KCube lives
    os.chdir(kcube_path)

    proc_time = None

    # invoke the KCube application with the supplied cube representation
    starttime = time.time()
    process = subprocess.Popen(["KCUBE", rep[0], rep[1], rep[2], rep[3], rep[4], rep[5]], shell=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)

    # poll the ongoing process for new output until finished
    solution_found = False
    sol_sequence = None
    while True:
        if solution_found:
            break

        nextline = process.stdout.readline()
        if nextline == '' and process.poll() != None:
            break

        for x in range(25):
            if "(" + str(25 - x) + ")" in nextline and not "threshold" in nextline:
                endtime  = time.time()
                proc_time = endtime - starttime
                sol_sequence = nextline
                solution_found = True
                break

    # if the sol_sequence is not empty, replace the period and remove the last 4 character assum 
    if(sol_sequence != None)
        sol_sequence = sol_sequence.replace(". ", "").replace("'", "b").split("(")[:-1]

    return (sol_sequence, proc_time)

if __name__ == '__main__':

    # create the CLI options
    parser = argparse.ArgumentParser(description='')
    parser.add_argument("--id1", required=True, help="The device ID for Camera 1.")
    parser.add_argument("--id2", required=True, help="The device ID for Camera 2.")
    parser.add_argument("--kthresh", required=True, help="The threshold value for Kociemba's algorithm.")
    parser.add_argument("--debug", "-d", action='store_true', help="Enable debugging.")

    args = parser.parse_args()

    id1 = args.id1
    id2 = args.id2
    debug = args.debug
    kcube_thresh = args.kthresh

    # get the Rubik's cube representation for Kociemba's algorithm by running
    # the image processing script
    cube_rep = get_cube_rep(id1, id2, debug)

    # get the solution sequence by invoking Kociemba's algorithm on the cube
    # representation
    kcube_path = "../Server/kcube"
    (sol_seq, proc_time) = get_cube_sol(cube_rep, kcube_path, kcube_thresh)

    if args.debug:
        print ("Solution Found: %s") % sol_seq
        print ("Process Time: %d") % proc_time
    
    sendCommand("ExecuteMoves " + sol_sequence)
