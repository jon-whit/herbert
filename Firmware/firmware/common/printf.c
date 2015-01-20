/////////////////////////////////////////////////////////////
//
//  printf.c
//
//  simple implementation of printf
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert


#include <stdio.h>
#include <stdarg.h>

extern void outbyte(char c); 


int printf(const char *msg, ...)
{
    #define BUF_SIZE 5000
    
    char buf[BUF_SIZE];
    
    va_list argp;
    
    va_start (argp, msg);
    //TODO: use vsnprintf to protect buffer
    vsprintf (buf, msg, argp);
    va_end (argp);
    
    int count;
    int i;
    for(i = 0, count = 0; i < BUF_SIZE; i++, count++)
    {
        char c = buf[i];
        
        if(!c)
            break;

        if(c == '\n')
        {
            outbyte('\r');
            count++;
        }
        outbyte(c);
    }
    
    return count;
}



int puts(const char *s)
{
    int count = 0;
    while(*s)
    {
        if(*s == '\n')
        {
            outbyte('\r');
            count++;
        }
        outbyte(*s++);
        count++;
    }
    
    outbyte('\r');
    outbyte('\n');
    
    return count + 2;
}

// EOF
