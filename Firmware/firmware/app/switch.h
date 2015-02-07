/////////////////////////////////////////////////////////////
//
//  switch.h
//


#ifndef switch_h
#define switch_h

#include <types.h>

typedef enum 
{
	LInSwitch,
	LOutSwitch,
	FInSwitch,
	FOutSwitch,
	RInSwitch,
	ROutSwitch,
	BInSwitch,
	BOutSwitch,
	UInSwitch,
	UOutSwitch,
	DInSwitch,
	DOutSwitch,
} SwitchLocation;


void InitializeSwitches();
bool IsSwitchTriggered(SwitchLocation switchLoc);

#endif
