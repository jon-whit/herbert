:: Xilinx make batchfile
@echo off


if not "%1" == "bin" goto firmware
:bin
echo Building Plate Cycler Firmware and binary image...
make -C ../ bin 1> make.log 2> make_err.log
goto end


:firmware
echo Building Plate Cycler Firmware...
make -C ../ firmware 1> make.log 2> make_err.log


:end

type make_err.log | sed -e "s;/cygdrive/c;C:;"
echo Done.
date /T
time /T
