/////////////////////////////////////////////////////////////
//
//  switch.c
//

#include <switch.h>
#include <xparameters.h>
#include <types.h>

///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    volatile uint32 SWITCH_L_IN_SIGNAL;    // L Face In Switch Signal    (1 bit)
	volatile uint32 SWITCH_L_IN_STIMULUS;  // L Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_L_OUT_SIGNAL;   // L Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_L_OUT_STIMULUS; // L Face Out Switch Stimulus (1 bit)
    volatile uint32 SWITCH_F_IN_SIGNAL;    // F Face In Switch Signal    (1 bit)
    volatile uint32 SWITCH_F_IN_STIMULUS;  // F Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_F_OUT_SIGNAL;   // F Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_F_OUT_STIMULUS; // F Face Out Switch Stimulus (1 bit)
    volatile uint32 SWITCH_R_IN_SIGNAL;    // R Face In Switch Signal    (1 bit)
    volatile uint32 SWITCH_R_IN_STIMULUS;  // R Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_R_OUT_SIGNAL;   // R Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_R_OUT_STIMULUS; // R Face Out Switch Stimulus (1 bit)
    volatile uint32 SWITCH_B_IN_SIGNAL;    // B Face In Switch Signal    (1 bit)
    volatile uint32 SWITCH_B_IN_STIMULUS;  // B Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_B_OUT_SIGNAL;   // B Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_B_OUT_STIMULUS; // B Face Out Switch Stimulus (1 bit)
    volatile uint32 SWITCH_U_IN_SIGNAL;    // U Face In Switch Signal    (1 bit)
    volatile uint32 SWITCH_U_IN_STIMULUS;  // U Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_U_OUT_SIGNAL;   // U Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_U_OUT_STIMULUS; // U Face Out Switch Stimulus (1 bit)
    volatile uint32 SWITCH_D_IN_SIGNAL;    // D Face In Switch Signal    (1 bit)
    volatile uint32 SWITCH_D_IN_STIMULUS;  // D Face In Switch Stimulus  (1 bit)
    volatile uint32 SWITCH_D_OUT_SIGNAL;   // D Face Out Switch Signal   (1 bit)
    volatile uint32 SWITCH_D_OUT_STIMULUS; // D Face Out Switch Stimulus (1 bit)

} SwitchRegs;


#define SWITCH (*((SwitchRegs*)XPAR_SWITCH_BASEADDR))

///////////////////////////////////////////////////
// Interface functions


void InitializeSwitches()
{
	// Stimulate the switches so triggers can be detected
	SWITCH.SWITCH_L_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_L_OUT_STIMULUS = 1; 
	SWITCH.SWITCH_F_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_F_OUT_STIMULUS = 1; 
	SWITCH.SWITCH_R_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_R_OUT_STIMULUS = 1; 
	SWITCH.SWITCH_B_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_B_OUT_STIMULUS = 1; 
	SWITCH.SWITCH_U_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_U_OUT_STIMULUS = 1; 
	SWITCH.SWITCH_D_IN_STIMULUS  = 1; 
	SWITCH.SWITCH_D_OUT_STIMULUS = 1; 
}

bool IsSwitchTriggered(SwitchLocation switchLoc)
{
	InitializeSwitches();
	if     (switchLoc == LInSwitch)  {return SWITCH.SWITCH_L_IN_SIGNAL; }
	else if(switchLoc == LOutSwitch) {return SWITCH.SWITCH_L_OUT_SIGNAL;}
	else if(switchLoc == FInSwitch)  {return SWITCH.SWITCH_F_IN_SIGNAL; }
	else if(switchLoc == FOutSwitch) {return SWITCH.SWITCH_F_OUT_SIGNAL;}
	else if(switchLoc == RInSwitch)  {return SWITCH.SWITCH_R_IN_SIGNAL; }
	else if(switchLoc == ROutSwitch) {return SWITCH.SWITCH_R_OUT_SIGNAL;}
	else if(switchLoc == BInSwitch)  {return SWITCH.SWITCH_B_IN_SIGNAL; }
	else if(switchLoc == BOutSwitch) {return SWITCH.SWITCH_B_OUT_SIGNAL;}
	else if(switchLoc == UInSwitch)  {return SWITCH.SWITCH_U_IN_SIGNAL; }
	else if(switchLoc == UOutSwitch) {return SWITCH.SWITCH_U_OUT_SIGNAL;}
	else if(switchLoc == DInSwitch)  {return SWITCH.SWITCH_D_IN_SIGNAL; }
	else if(switchLoc == DOutSwitch) {return SWITCH.SWITCH_D_OUT_SIGNAL;}
	else {return false;}
	return false;
	return 0;
}

// EOF
