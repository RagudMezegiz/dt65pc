# DT65PC

Dave Terhune's 65xx Personal Computer

This is a non-existent computer that could theoretically be built today,
although some components may need to be sourced from eBay.

This computer uses a WDC 65C816 microprocessor at its core, with support
chips selected for both ease of use and potential acquirability.

* Main CPU: 65C816
* Video: V9938

### Building the kernel ROM

Requirements:

* 65816 assembler
  * Initial builds use ca65
  * Once implemented, native assembler
* GNU-compatible make program

## Memory Map

Bank 00:
* 0000-CFFF = RAM
* D000-D7FF = Memory-mapped IO
* D800-DFFF = Character font ROM
* E000-FFFF = Kernel ROM

Banks 01-DF:
* 0000-FFFF = RAM

Banks E0-FF:
* Math table ROMS from Garth Wilson

## System Calls

The kernel provides support facilities through a system call interface,
where parameters are held in the X and Y registers, and the system call
number in A, and a COP instruction issued.  System call numbers TBD.

## Shell Interface

A Unix-like shell interface is planned.  Details TBD.

## Utilities

The following utilities are planned:

* Monitor with assembler and disassembler
* A full-fledged symbolic assembler
  * Cross-assembler implemented in C
  * Native version
* Higher level language interpreter/compiler

