# Makefile for DT65PC ROM images
# Copyright (c) 2019 David Terhune

all: dt65pc.rom

dt65pc.rom: dt65pc.o
	ld65 -C dt65pc.cfg -S 57344 -o $@ $<

dt65pc.o: dt65pc.s dt65pc.inc v9938.inc
	ca65 --cpu 65816 -l dt65pc.lst $<

.PHONY: clean

clean:
	rm *.o *.rom *.lst

