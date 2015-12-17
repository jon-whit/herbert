The "ImageProcessing" directory contains all the source code and files used in the
image processing and interfacing with the FPGA system.

This README outlines what each file is being used for in the overall system.

--------
main.py
--------
main.py contains our main server side program that performs capturing cube state,
image processing, finding solve sequence, and send over solution sequence to FPGA.

Example: python main.py --id 0 

 --id               - The device ID for top Camera.
 --disableOptimized - Disables image processing optimization
 --sequenceMode     - Execute all solving move sequence in one ExecuteCommand 
 --debug            - Enable debugging prints
 --t_thresh         - Amount of time to wait for KCUBE to find optimized solution
 --port             - The COM port which the FPGA is plugged into.

----------------
commInterface.py
----------------
commInterface.py contains all the code used to communicate serially with the FPGA.
commInterface.py is used by main.py and should be in the same directory. Performs
CRC32 calculations and connects to the FPGA serially.

---------------
kcube directory
---------------
Contains 3rd party code provided by Kociemba. The source of this file is can be found
in our document.

----------------------
Additional *.txt files
----------------------
Contains all the static polygon mask for each of the facelet and is used by
main.py for image capturing.

----------------------------
HerbertCommInterfaceGUI.py/ui
----------------------------
Used to manually communicate serially with the FPGA. It is used for debugging one
the server side with a GUI.

---------------
test_points.py
---------------
test_points.py is used as a helper python script to test each of the masks defined
for each facelet. Through this script, we can quickly visualize if all the mask 
are correctly aligned.

Example: python test_points.py -p mask_file.txt -d image_directory

---------------
cube_capture.py
---------------
cube_capture.py contains prototype code for dynamic cube state detection. 

---------
main.cpp
---------
main.cpp contains old image processing code in C++ before we had to port to python.

---------
*.bmp
---------
Some example images we taken during a solve sequence