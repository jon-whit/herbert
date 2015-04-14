/////////////////////////////////////////////////////////////
//
//  lcd.c
//
//  Xilinx Spartan-3A FPGA Starter Kit LCD Driver
//

#include <lcd.h>
#include <xparameters.h>
#include <stdio.h>
#include <stdarg.h>
#include <timer.h>



///////////////////////////////////////////////////
// Constants

//Commands
#define LCD_CMD_CLEAR_DISP      0x01
#define LCD_CMD_RETURN_HOME     0x02
#define LCD_CMD_ENTRY_MODE_SET  0x04
#define LCD_CMD_DISPLAY_ON_OFF  0x08
#define LCD_CMD_SHIFT           0x10
#define LCD_FUNCTION_SET        0x38  // 8-bit, 2-lines, 5x8 format
#define LCD_CMD_SET_CGRAM_ADDR  0x40
#define LCD_CMD_SET_DGRAM_ADDR  0x80

//Masks
#define ENTRY_MODE_INC_MASK     0x02
#define ENTRY_MODE_SHIFT_MASK   0x01

#define DISPLAY_MASK            0x04
#define DISPLAY_CURSOR_MASK     0x02
#define DISPLAY_CURSOR_POS_MASK 0x01

#define SHIFT_CURSOR_MASK       0x08
#define SHIFT_DISPLAY_MASK      0x04

#define BUSY_MASK               0x80

#define DDRAM_ADDR_MASK         0x7F
#define CGRAM_ADDR_MASK         0x3F



///////////////////////////////////////////////////
// Local types and macros

typedef struct
{
    volatile uint32 rw;
    volatile uint32 rs;
    volatile uint32 enable;
    volatile uint32 data;
} LcdRegs;


#define LCD (*((LcdRegs*)XPAR_LCD_BASEADDR))



///////////////////////////////////////////////////
// Local function prototypes

static void lcdSetEnable();
static void lcdClrEnable();
static void lcdPulseEnable();
static void setDdramAddr(uint8 addr);
static void sendCommand(uint8 command);
static void sendData(uint8 data);
static bool lcdBusy();
static void waitForIdle();



///////////////////////////////////////////////////
// Local data

static bool lcdPresent = true;



///////////////////////////////////////////////////
// Interface functions

void lcdInit()
{
    LCD.enable = CLR;
    LCD.rw     = CLR;
    LCD.rs     = CLR;
    LCD.data   = 0x00;

    udelay(40000);

    LCD.data = LCD_FUNCTION_SET;
    lcdPulseEnable();

    udelay(37);
    
    LCD.data = LCD_FUNCTION_SET;
    lcdPulseEnable();

    udelay(37);
    
    LCD.data = LCD_CMD_DISPLAY_ON_OFF | DISPLAY_MASK;
    lcdPulseEnable();

    udelay(37);
    
    LCD.data = LCD_CMD_CLEAR_DISP;
    lcdPulseEnable();

    udelay(1520);
    
    LCD.data = LCD_CMD_ENTRY_MODE_SET | ENTRY_MODE_INC_MASK;
    lcdPulseEnable();
}



void lcdClear()
{
    sendCommand(LCD_CMD_CLEAR_DISP);
}



void lcdLineDisplay(uint8 line, char* text)
{
    if(!lcdPresent)
    {
        return;
    }

    int i;
    
    if(line > 1)
    {
        return;
    }
    
    if(line == 0)
    {
        setDdramAddr(0);
    }
    else
    {
        setDdramAddr(40);
    }
    
    for(i = 0; i < LCD_LINE_LENGTH; i++)
    {
        if(*text)
        {
            sendData(*text++);
        }
        else
        {
            sendData(' ');
        }
    }
}



int lcdLinePrintf(uint8 line, const char *msg, ...)
{
    #define BUF_SIZE 200
    
    int result;
    char buf[BUF_SIZE];
    
    va_list argp;
    
    va_start (argp, msg);
    //TODO: use vsnprintf to protect buffer
    result = vsprintf (buf, msg, argp);
    va_end (argp);
    
    printf("LCD: '%s'\n", buf);
    lcdLineDisplay(line, buf);
    return result;
}



///////////////////////////////////////////////////
// Local functions

static void lcdSetEnable()
{
    LCD.enable = SET;
    udelay(12);
}



static void lcdClrEnable()
{
    LCD.enable = CLR;
}



static void lcdPulseEnable()
{
    lcdSetEnable();
    lcdClrEnable();
}



static void setDdramAddr(uint8 addr)
{
    sendCommand(LCD_CMD_SET_DGRAM_ADDR | (addr & DDRAM_ADDR_MASK));
}



static void sendCommand(uint8 command)
{
    waitForIdle();
    
    LCD.enable = CLR;
    LCD.rw     = CLR;
    LCD.rs     = CLR;
    LCD.data   = command;
    
    lcdPulseEnable();
}



static void sendData(uint8 data)
{
    waitForIdle();
    
    LCD.rs     = SET;

    LCD.data   = data;
    
    lcdPulseEnable();

    LCD.rs     = CLR;
}



static bool lcdBusy()
{
    uint8 data;
    
    LCD.rw = SET;

    lcdSetEnable();
    
    data = LCD.data;
    
    lcdClrEnable();
    LCD.rw = CLR;

    return !!(data & BUSY_MASK);
}



static void waitForIdle()
{
    int count = 0;
    
    while(lcdPresent && lcdBusy())
    {
        if(++count > 100)
        {
            lcdPresent = false;
            break;
        }
        
        udelay(37);
    }
}
