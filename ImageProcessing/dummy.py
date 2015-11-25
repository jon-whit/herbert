import argparse
import os
import signal
import time
import subprocess

def print_facelets(id1, debug):
    # call program to get the facelets we need
    if debug:
        cmd = "./dummy {0} -d".format(id1)
    else:
        cmd = "./dummy {0}".format(id1)

    process = subprocess.Popen([cmd], shell=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)

    # print the outprints
    print(process.stdout.readline())

if __name__ == '__main__':
    # create the CLI options
    parser = argparse.ArgumentParser(description='')
    parser.add_argument("--id1", required=True, help="The device ID for Camera 1.")
    parser.add_argument("--debug", "-d", action='store_true', help="Enable debugging.")
    args = parser.parse_args()
    id1 = args.id1
    debug = args.debug

    print_facelets(id1, debug)
