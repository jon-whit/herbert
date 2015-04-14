/////////////////////////////////////////////////////////////
//
//  lcd.h
//
//  Xilinx Spartan-3A FPGA Starter Kit LCD Driver
//

#ifndef lcd_h
#define lcd_h

#include <types.h>


#define LCD_LINE_LENGTH 16



void lcdInit();
void lcdClear();

void lcdLineDisplay(uint8 line, char* text);
int  lcdLinePrintf(uint8 line, const char *msg, ...);

#endif
