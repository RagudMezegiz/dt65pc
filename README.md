# DT65PC

Dave Terhune's 65xx Personal Computer

This is a general-purpose personal computer intended to evoke feelings
of nostalgia over the simpler computers of my youth, only with a boost
in capabilities over the 8-bit machines I used back then by using a
16-bit processor. This computer could theoretically be built today and
some day I may even do so. Until then, however, the hardware will be
simulated.

## Hardware

This computer uses a WDC 65C816 microprocessor at its core, with support
chips selected for both ease of use and potential acquirability.

* Main CPU: 65C816
* Serial I/O: PC16550D UART
* Utility: 65C22 VIA

### Memory Map

Bank 00:

* 0000-CFFF = RAM
* D000-DFFF = Memory-mapped IO
* E000-FFFF = Kernel ROM

Banks 01-DF:

* 0000-FFFF = RAM

Banks E0-FF:

* Math table ROMS from Garth Wilson

## Kernel

### Building the kernel ROM

Requirements:

* 65816 assembler
  * Initial builds use ca65
  * Once implemented, native assembler
* GNU-compatible make program

### System Calls

The kernel provides support facilities through a system call interface,
where parameters are held in the X and Y registers, and the system call
number in A, and a COP instruction issued.  System call numbers TBD.

### Shell Interface

A Unix-like shell interface is planned.  Details TBD.

## Utilities

The following utilities are planned:

* Monitor with assembler and disassembler
* A full-fledged symbolic assembler
  * Cross-assembler implemented in an HLL
  * Native version
* Higher level language interpreter/compiler
