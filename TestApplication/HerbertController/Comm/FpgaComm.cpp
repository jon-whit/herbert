#include <FpgaComm.h>


FpgaComm::FpgaComm()
    : Comm(FPGA_COMM_BAUD_RATE) 
{
}


FpgaComm& FpgaComm::getInstance()
{
    static FpgaComm instance;
    return instance;
}


