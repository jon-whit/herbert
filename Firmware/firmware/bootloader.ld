/*******************************************************************/
/*                                                                 */
/* This file is automatically generated by linker script generator.*/
/*                                                                 */
/* Version: Xilinx EDK 10.1.03 EDK_K_SP3.6                                */
/*                                                                 */
/* Copyright (c) 2004 Xilinx, Inc.  All rights reserved.           */
/*                                                                 */
/* Description : MicroBlaze Linker Script                          */
/*                                                                 */
/*******************************************************************/

_SDRAM_SIZE       = 0x04000000;
_SCRATCH_RAM_SIZE = 0x03000000;
_STACK_SIZE       = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x4000;
_HEAP_SIZE        = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x4000;

/* Define Memories in the system */

MEMORY
{
   ilmb_cntlr_dlmb_cntlr : ORIGIN = 0x00000050, LENGTH = 0x0007FB0
   DDR2_SDRAM_C_MPMC_BASEADDR : ORIGIN = 0x90000000, LENGTH = 0x04000000
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0x00000000 : {
   PROVIDE(__vectors = .);
   *(.vectors.reset)
} 

.vectors.sw_exception 0x00000008 : {
   *(.vectors.sw_exception)
} 

.vectors.interrupt 0x00000010 : {
   *(.vectors.interrupt)
} 

.vectors.hw_exception 0x00000020 : {
   *(.vectors.hw_exception)
} 

.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > ilmb_cntlr_dlmb_cntlr

.init : {
   KEEP (*(.init))
} > ilmb_cntlr_dlmb_cntlr

.fini : {
   KEEP (*(.fini))
} > ilmb_cntlr_dlmb_cntlr

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > ilmb_cntlr_dlmb_cntlr

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > ilmb_cntlr_dlmb_cntlr

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > ilmb_cntlr_dlmb_cntlr

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > ilmb_cntlr_dlmb_cntlr

.got : {
   *(.got)
} > ilmb_cntlr_dlmb_cntlr

.got1 : {
   *(.got1)
} > ilmb_cntlr_dlmb_cntlr

.got2 : {
   *(.got2)
} > ilmb_cntlr_dlmb_cntlr

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > ilmb_cntlr_dlmb_cntlr

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > ilmb_cntlr_dlmb_cntlr

.eh_frame : {
   *(.eh_frame)
} > ilmb_cntlr_dlmb_cntlr

.jcr : {
   *(.jcr)
} > ilmb_cntlr_dlmb_cntlr

.gcc_except_table : {
   *(.gcc_except_table)
} > ilmb_cntlr_dlmb_cntlr

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > ilmb_cntlr_dlmb_cntlr

.sbss : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > ilmb_cntlr_dlmb_cntlr

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > ilmb_cntlr_dlmb_cntlr

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > ilmb_cntlr_dlmb_cntlr

/* Generate Stack and Heap definitions */

.scratch : {
   /*. = ALIGN(32);*/
   _scratch_start = .;
   PROVIDE(__scratch_start = .); 
   *(.scratch)
   . += _SCRATCH_RAM_SIZE;
   _scratch_end = .;
   PROVIDE (__scratch_end = .);
} > DDR2_SDRAM_C_MPMC_BASEADDR

PROVIDE (__scratch_size = (_scratch_end - _scratch_start + 1));

.bss : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > DDR2_SDRAM_C_MPMC_BASEADDR

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

.heap : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > DDR2_SDRAM_C_MPMC_BASEADDR

.stack : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > DDR2_SDRAM_C_MPMC_BASEADDR

}

