/////////////////////////////////////////////////////////////
//
//  conversion.c
//
//  Conversion Utilities


#include <conversion.h>
#include <string.h>


uint32 wordsToDWord(uint16 high, uint16 low)
{
    uint32 value;
    uint16 words[2];

    words[0] = high;
    words[1] = low;

    memcpy(&value, words, 4);

    return value;
}


void uint16ToAsciiHex( char *str, uint16 val )
{
    if( str )
    {
        int i;

        for( i = 3; i >= 0; --i )
        {
            uint8 nibble = ( uint8 )( val >> ( i << 2 ) ) & 0x0F;

            if( nibble < 10 )
            {
                nibble += '0';
            }
            else
            {
                nibble -= 10;
                nibble += 'A';
            }

            *str++ = nibble;
        }

        *str = '\0';
    }
}


// EOF
