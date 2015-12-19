This folder contains the firmware for the instrument. 

firmware 
	- contains the bootloader and application firmware
	- app 
		-contains the application code (we wrote most of this code)
	- bootloader 
		-contains the bootloader code (we did not modify this)
	- common 
		-contains code common to bootloader and application (we did not modify this)
       - firmware-original.zip
		-contains a clean copy of the original source code that we modified

fpga
	- contains all of the vhdl code for the fpga (we added all vhdl code for our peripherals)

third_party
	- contains third party libraries used for the microblaze processor (third party)


FirmwareInstructions.txt
	- Instructions on how to build the firmware using cygwin

Makefile
	- The makefile for making the build artifacts (we modified this for our build)

The other files in this folder are build artifacts for the fpga image and the firmware application image.


