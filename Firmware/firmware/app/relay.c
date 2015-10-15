/////////////////////////////////////////////////////////////
//
//  relay.c
//

#include <relay.h>
#include <xparameters.h>
#include <types.h>

#define OFF 1
#define ON  0

///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    volatile uint32 relay_F; // F Face Air Cylinder (1 bit)
    volatile uint32 relay_R; // R Face Air Cylinder (1 bit)
    volatile uint32 relay_D; // D Face Air Cylinder (1 bit)
    volatile uint32 relay_U; // U Face Air Cylinder (1 bit)
    volatile uint32 relay_B; // B Face Air Cylinder (1 bit)
    volatile uint32 relay_L; // L Face Air Cylinder (1 bit)
} RelayRegs;


#define RELAY (*((RelayRegs*)XPAR_RELAY_BASEADDR))

///////////////////////////////////////////////////
// Interface functions


void InitializeAirCylinders()
{
	RELAY.relay_U = OFF;
	RELAY.relay_F = OFF;
	RELAY.relay_R = OFF;
	RELAY.relay_D = OFF;
	RELAY.relay_B = OFF;
	RELAY.relay_L = OFF;
}

void ActuateArmIn(char armFace)
{
	InitializeAirCylinders();
	if(armFace == 'U')      { RELAY.relay_U = ON; ;}
	else if(armFace == 'F') { RELAY.relay_F = ON; }
	else if(armFace == 'R') { RELAY.relay_R = ON; }
	else if(armFace == 'D') { RELAY.relay_D = ON; }
	else if(armFace == 'B') { RELAY.relay_B = ON; }
	else if(armFace == 'L') { RELAY.relay_L = ON; }
	else {/*Do Nothing*/}

}

void ActuateArmsOut()
{
	InitializeAirCylinders();
}

// EOF
