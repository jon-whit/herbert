:: Xilinx make batchfile
@echo off

echo Programming FPGA Platform Flash...


make -C ../ program_prom 1> make.log 2> make_err.log


:end

type make_err.log | sed -e "s;/cygdrive/c;C:;"
echo Done.
date /T
time /T
