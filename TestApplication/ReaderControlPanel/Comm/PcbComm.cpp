#include <PcbComm.h>


PcbComm::PcbComm()
    : Comm(PCB_COMM_BAUD_RATE) 
{
}


PcbComm& PcbComm::getInstance()
{
    static PcbComm instance;
    return instance;
}


