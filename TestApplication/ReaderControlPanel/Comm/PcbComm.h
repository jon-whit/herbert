

#ifndef PrbComm_h
#define PrbComm_h

#include <Comm.h>


#define PCB_COMM_BAUD_RATE CSerial::EBaud115200



class PcbComm : public Comm
{
public:
    static PcbComm& getInstance();

private:
    PcbComm();
    PcbComm(PcbComm&);
    PcbComm& operator=(PcbComm&);
};

#endif