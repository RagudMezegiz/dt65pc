; DT65PC Yamaha v9938 interface
; Copyright (c) 2019 David Terhune
;
; ca65 assembler syntax version

; Yamaha v9938 base address in RAM.  Note that if the direct page register is
; set to D000, direct page addresses are the port numbers (no constants).
v9938_base = $D000

; Register copy base address.  Add register number to get register address.
v9938_rcopy = $CF00

; VRAM base addresses
v9938_screen = $0000
v9938_color  = $0A00
v9938_font   = $1000

; Bit flags
v9938_BL = $40

; Write a byte of data to a v9938 register using absolute addressing.
; Disable interrupts before this macro call, and re-enable afterward.
.macro v9938_setreg D, R
    lda #D
    sta v9938_rcopy + R
    sta v9938_base + 1
    lda #R + 128
    sta v9938_base + 1
.endmac

; Write a byte of data to a v9938 register using direct page addressing.
; The direct page register must be set to D000.  Disable interrupts before
; this macro call, and re-enable afterward.
.macro v9938_setreg_dp D, R
    lda #D
    sta v9938_rcopy + R
    sta 1
    lda #R + 128
    sta 1
.endmac

; Disable the screen.  Uses the copy of R#1, clears the screen enable bit,
; and writes it.  Disable interrupts before this macro call, and re-enable
; afterward.
.macro v9938_disable_screen
    lda v9938_rcopy + 1
    and #<~v9938_BL
    sta v9938_rcopy + 1
    sta v9938_base + 1
    lda 129
    sta v9938_base + 1
.endmac

; Disable the screen.  Uses the copy of R#1, clears the screen enable bit, and
; writes it.  The direct page register must be set to V9938 base address.
; Disable interrupts before this macro call, and re-enable afterward.
.macro v9938_disable_screen_dp
    lda v9938_rcopy + 1
    and #<~v9938_BL
    sta v9938_rcopy + 1
    sta 1
    lda 129
    sta 1
.endmac

; Enable the screen.  Uses the copy of R#1, sets the screen enable bit, and
; writes it.  Disable interrupts before this macro call, and re-enable
; afterward.
.macro v9938_enable_screen
    lda v9938_rcopy + 1
    ora #v9938_BL
    sta v9938_rcopy + 1
    sta V9938 + 1
    lda 129
    sta V9938 + 1
.endmac

; Enable the screen.  Uses the copy of R#1, sets the screen enable bit, and
; writes it.  The direct page register must be set to V9938 base address.
; Disable interrupts before this macro call, and re-enable afterward.
.macro v9938_enable_screen_dp
    lda v9938_rcopy + 1
    ora #v9938_BL
    sta v9938_rcopy + 1
    sta 1
    lda 129
    sta 1
.endmac

; Write a character (in 8-bit accumulator) to screen address (in 16-bit X
; register).  Does some wonky bit-shifting to get the address set correctly.
; Gets away with only doing bit-shifting because the screen base address is
; 0000.  Don't ever move the screen to a different base address!
.macro v9938_write_char
    ; Put the address in temporary storage so it can be bit-shifted.
    stx k_temp_0
    ; Zero out temp 3 so bits can be shifted into it.
    stz k_temp_3
    ; Shift the upper two bits out of the upper word and into temp 3.
    asl k_temp_1
    rol k_temp_2
    asl k_temp_1
    rol k_temp_2
    ; Set carry so the write bit will be set after shifting temp 2 back.
    sec
    ror k_temp_1
    lsr k_temp_1
    ; Save accumulator
    pha
    ; Write highest bits to register 14
    lda k_temp_3
    sta v9938_rcopy + 14
    sta v9938_base + 1
    lda #142
    sta v9938_base + 1
    ; Write low byte of screen address then middle bits and write flag
    lda k_temp_0
    sta v9938_base + 1
    lda k_temp_1
    sta v9938_base + 1
    ; Now write the character byte
    pla
    sta v9938_base
.endmac

