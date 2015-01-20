/////////////////////////////////////////////////////////////
//
//  stubs.c
//
//  Function stubs for functions that are linked in but
//  not used in the bootloader
//
//  Copyright 2009 Idaho Technology
//  Created by Brett Gilbert

#include <stdio.h>
#include <sys/stat.h>
#include <mb_interface.h>

int fflush(FILE *FP)                                         {return 0;}
int fstat(int fildes, struct stat *buf)                      {return 0;}

ssize_t write(int fildes, const void *buf, size_t nbyte)     {return 0;}

int _puts_r(struct _reent *REENT, const char *S)             {return 0;}

void *malloc(size_t NBYTES)                                  {return NULL;}
void *realloc(void *APTR, size_t NBYTES)                     {return NULL;}
void free(void *APTR)                                        {}
void *memalign(size_t ALIGN, size_t NBYTES)                  {return NULL;}
size_t malloc_usable_size(void *APTR)                        {return 0;}
void *_malloc_r(void *REENT, size_t NBYTES)                  {return NULL;}
void *_realloc_r(void *REENT, void *APTR, size_t NBYTES)     {return NULL;}
void _free_r(void *REENT, void *APTR)                        {}
void *_memalign_r(void *REENT, size_t ALIGN, size_t NBYTES)  {return NULL;}
size_t _malloc_usable_size_r(void *REENT, void *APTR)        {return 0;}

//void microblaze_register_handler(XInterruptHandler Handler, void *DataPtr) {}



// EOF
