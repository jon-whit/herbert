

#ifndef FpgaComm_h
#define FpgaComm_h

#include <Comm.h>


#define FPGA_COMM_BAUD_RATE CSerial::EBaud115200



class FpgaComm : public Comm
{
public:
    static FpgaComm& getInstance();

private:
    FpgaComm();
    FpgaComm(FpgaComm&);
    FpgaComm& operator=(FpgaComm&);
};

#endif