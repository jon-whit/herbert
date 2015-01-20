/* Simple sprintf
Copyright (C) 2001 Free Software Foundation, Inc.
Written by Stephane Carrez (stcarrez@worldnet.fr)

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

*/

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>

#define HEX_CVT_MASK  (0x0fffffffL)

static char *
hex_convert (char *buf, unsigned long value, char padChar, int padCount)
{
    char num[32];
    int pos;

    pos = 0;
    while (value != 0)
    {
        char c = value & 0x0F;
        num[pos++] = "0123456789ABCDEF"[(unsigned) c];
        value = (value >> 4) & HEX_CVT_MASK;
    }
    if (pos == 0)
        num[pos++] = '0';

    while (padCount-- - pos > 0)
        *buf++ = padChar;
        
    
    while (--pos >= 0)
        *buf++ = num[pos];

    *buf = 0;
    return buf;
}

static char *
dec_convert (char *buf, long value)
{
    char num[20];
    int pos;

    pos = 0;
    if (value < 0)
    {
        *buf++ = '-';
        value = -value;
    }
    while (value != 0)
    {
        char c = value % 10;
        value = value / 10;
        num[pos++] = c + '0';
    }
    if (pos == 0)
    num[pos++] = '0';

    while (--pos >= 0)
    {
        *buf = num[pos];
        buf++;
    }
    *buf = 0;
    return buf;
}

int strtoint(const char *s, char **endptr)
{
    char *p     = (char*)s;
    int   value = 0;

    while(*p && isdigit(*p))
    {
        value *= 10;
        value += (*p) - '0';
        p++;
    }
    
    *endptr = p;
    
    return value;
}


/* A very simple vsprintf (from calc.c). It only recognizes %d and %x.
The parameter MUST be of type 'long'. Otherwise, you will not get
the expected result! */
int
vsprintf (char *buf, const char *pattern, va_list argp)
{
    char *p = buf;
    char c;

    while ((c = *pattern++) != 0)
    {
        if (c != '%')
        {
            *p++ = c;
        }
        else
        {
            long v;
            char longType = 0;
            char padChar  = ' ';
            int  padCount;
            char* ppad;

            padCount = strtoint(pattern, &ppad);
            
            if(pattern != ppad)
            {
                if (*pattern == '0')
                {
                    padChar = '0';
                }
            
                pattern = ppad;
            }
            
            c = *pattern++;

            if (c == 'l')
            {
                c = *pattern++;
                longType = 1;
            }

            switch (c)
            {
                case 'b':
                case 'o':
                case 'x':
                    if(longType)
                    {
                        v = va_arg (argp, long);
                    }
                    else
                    {
                        v = va_arg (argp, int) & 0x0000ffff;
                    }
                    p = hex_convert (p, v, padChar, padCount);
                    break;

                case 'd':
                case 'u':
                    if(longType)
                    {
                        v = va_arg (argp, long);
                    }
                    else
                    {
                        v = va_arg (argp, int);
                    }
                    p = dec_convert (p, v);
                    break;

                case 'c':
                    c = va_arg (argp, int);
                    *p++ = c;
                    break;

                case 's':
                    {
                        const char *q = va_arg (argp, const char*);
    
                        if (q == 0)
                            q = "(null)";
                        while ((*p++ = *q++) != 0)
                            continue;
                        p--;
                    }
                    break;

                case '%':
                    *p++ = '%';
                    break;

                default:
                    *p++ = '%';
                    *p++ = c;
                    break;
            }
        }
    }
    *p++ = 0;
    return (size_t) (p - buf);
}

int
sprintf (char *buf, const char *msg, ...)
{
    int result;
    va_list argp;

    va_start (argp, msg);
    result = vsprintf (buf, msg, argp);
    va_end (argp);

    return result;
}

