#include "crc16.h"


#define CRCCCITT_TABLE_SIZE 256
#define P_CCITT             0x1021


static unsigned short crc_tabccitt[CRCCCITT_TABLE_SIZE]; 
static int            CRC_tabccitt_init = 0;



///////////////////////////////////
//	*** Calculate CRC Table ***
///////////////////////////////////
void initCRC()
{
    int i, j;
    unsigned short crc, c;

    for(i = 0; i < CRCCCITT_TABLE_SIZE; i++) 
	{
        crc = 0;
        c   = ((unsigned short) i) << 8;
        for (j=0; j<8; j++) 
		{
            if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ P_CCITT;
            else                      crc =   crc << 1;

            c = c << 1;
        }
        crc_tabccitt[i] = crc;
    }
    CRC_tabccitt_init = 1;
}



///////////////////////////////////
//	*** Calculate CRC ***
///////////////////////////////////
// Calculate CCITT standard 16-bit CRC
// (which uses polynomial x^16 + x^12 + x^5 + 1)
// This version actually works
CRC16 calcCRC16(CRC16 crc, const char *pStr, size_t lenStr)
{		
	int tmp,i;
	char nextchar;

	if ( ! CRC_tabccitt_init ) initCRC();
	
	for(i=0;i<(int)lenStr;i++)
	{	
		nextchar = *pStr;
		tmp = ((nextchar ^ (crc >> 8)) & 0xFF);
		crc = ((crc << 8) ^ crc_tabccitt[tmp])&0xFFFF;
		pStr++;
	}
    return crc;

}

