# Makefile for DT65PC ROM images
# Copyright (c) 2019-2023 David Terhune

S_FILES=$(wildcard *.s)

all: dt65pc.rom rom0.rom rom1.rom

dt65pc.rom: dt65pc.o dt65pc.cfg
	ld65 -C dt65pc.cfg -S 49152 -o $@ $<

dt65pc.o: $(S_FILES)
	ca65 --cpu 65816 -l dt65pc.lst $<

rom0.rom: ROM0.HEX
	objcopy -I ihex -O binary $< $@

rom1.rom: ROM1.HEX
	objcopy -I ihex -O binary $< $@

.PHONY: clean

clean:
	rm *.o *.rom *.lst

