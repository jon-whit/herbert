import subprocess
import time
import signal
import os
import sys
import psutil

def killProcess(process):
    children = psutil.Process(process.pid).children(recursive = True)
    for child in children:
        child.kill()

directory   = "C:\\Git\\herbert\\Server\\kcube\\"

os.chdir(directory)

cubeConfig1 = "U:BORGYBBOO D:WWWYWGROR F:WBYORWGBG B:WGYBORYWY L:OYOGBYGRR R:GRBRGYOWB"

starttime = time.time()

process = subprocess.Popen(["KCUBE", "U:BORGYBBOO", "D:WWWYWGROR", "F:WBYORWGBG", "B:WGYBORYWY", "L:OYOGBYGRR", "R:GRBRGYOWB"], shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

solutionFound = False

# Poll process for new output until finished
while True:
    if solutionFound:
        break
    nextline = process.stdout.readline()
    if nextline == '' and process.poll() != None:
        break
    for x in range(25):
        if "(" + str(25 - x) + ")" in nextline and not "threshold" in nextline:
            endtime   = time.time()
            fullTime  = endtime - starttime
            print "Solution - " + nextline
            print "Time - "     + str(fullTime)
            solutionFound = True
killProcess(process)
        







