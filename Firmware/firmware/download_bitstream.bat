::::::::::::::::::::::::::::::::::::::
::
::  Download bitstream to FPGA
::
::  Created by Brett C. Gilbert
::


@echo off

echo Download Bitstream to MFC...

xbash -q -c "/usr/bin/make -f system.make download; exit;"

echo Done.
date /T
time /T
