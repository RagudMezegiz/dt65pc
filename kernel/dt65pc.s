; DT65PC kernel source code
; Copyright (c) 2019 David Terhune
;
; ca65 assembler syntax version

; Force 65816 mode in case the command-line invocation didn't
.p816

; Kernel ROM starts at E000
    .org $E000

; Macro includes
.include "dt65pc.inc"
.include "v9938.inc"

; Highest RAM address
k_maxram = $DFFFFF

; Stack top of AFFF leaves 8K kernel RAM and user direct page (B000-CFFF)
k_stack_top = $AFFF

; Character font ROM address - occupies upper half of I/O region
k_font_addr = $D800

; Screen constants not in the v9938 include file
k_cursor_ymax = 24

; Math ROM base addresses
k_math_square   = $E00000
k_math_invert   = $E40000
k_math_sin      = $E80000
k_math_asin     = $EA0000
k_math_atan     = $EC0000
k_math_log2     = $ED0000
k_math_bitrev14 = $EF0000
k_math_bitrev13 = $EF8000
k_math_bitrev12 = $EFC000
k_math_bitrev11 = $EFE000
k_math_bitrev10 = $EFF000
k_math_bitrev09 = $EFF800
k_math_bitrev08 = $EFFC00
k_math_bitrev07 = $EFFE00
k_math_bitrev06 = $EFFF00
k_math_bitrev05 = $EFFF80
k_math_bitrev04 = $EFFFC0
k_math_bitrev03 = $EFFFE0
k_math_bitrev02 = $EFFFF0
k_math_alog2    = $F00000
k_math_log2a    = $F20000
k_math_alog2a   = $F40000
k_math_log2b    = $F60000
k_math_alog2b   = $F80000
k_math_sqrt1    = $FA0000
k_math_sqrt2    = $FB0000
k_math_sqrt3    = $FC0000
k_math_mult     = $FE0000

; Kernel direct page storage (assuming direct page register is set to C000)
k_bytes_free = 0        ; Free bytes count, 32 bits
k_cursor_xpos = 4       ; Cursor X position, 16 bits
k_cursor_ypos = 6       ; Cursor Y position, 16 bits
k_temp_0 = $70          ; Temporary working storage. No method can assume
k_temp_1 = $71          ; any value in temporary storage is preserved.
k_temp_2 = $72          ; Any subroutine has permission to muck about with
k_temp_3 = $73          ; any and/or all temporary storage locations.  That
k_temp_4 = $74          ; being said, most methods only need the first few
k_temp_5 = $75          ; addresses - it's not guaranteed, but you might be
k_temp_6 = $76          ; able to get away with storing stuff in values A-F
k_temp_7 = $77          ; and have a reasonable chance they'll still be there
k_temp_8 = $78          ; when you need them.
k_temp_9 = $79
k_temp_A = $7A
k_temp_B = $7B
k_temp_C = $7C
k_temp_D = $7D
k_temp_E = $7E
k_temp_F = $7F
k_keybuf_size = $80     ; Number of characters in keyboard buffer
k_keybuf = $81          ; Start of keyboard buffer (127 characters total)

; Kernel API call jump table.  All API calls are to two-byte addresses
; residing in kernel ROM.

    .res $E100 - *, $00  ; leave all 128 jump table slots free

; Kernel string constant section.
msg_welcome:
    .asciiz "* * * Welcome to the DT65PC! * * *"

; Emulation mode vectors.  Emulation mode is not supported, so all emulation
; mode handlers generate a fatal error and halt the system.
; TODO Investigate whether it makes more sense to perform a soft reset instead.
emul_cop:
emul_abortb:
emul_nmib:
emul_irq:
    ; Print error message on screen?
    stp

; Hard reset handler
.proc emul_resetb
    .a8
    ; Switch to native mode
    clc
    xce
    ; Set stack pointer
    set16i
    ldx #k_stack_top
    txs
    ; Set direct page to normal kernel location
    pea $C000
    pld
    ; Memory test
    jsr k_mem_test
    ; Initial screen setup
    jsr k_screen_init
    ; Copy character font to VRAM
    jsr k_font_copy
    ; Restore kernel direct page
    pea $C000
    pld
    ; Initialize cursor position and keyboard buffer
    jsr k_cursor_init
    ; Print welcome message
    ldx #msg_welcome
    jsr k_puts
    jsr k_newline
    ; Print free memory
    
    ; Check for filesystem
    
    ; If filesystem, load operating system
    
    ; Else start monitor
    jmp mon_start
.endproc

; Initialize cursor position to upper-left corner of screen and clear
; keyboard buffer.  Assumes 8-bit accumulator and direct page assigned
; to C000.
.proc k_cursor_init
    .a8
    stz k_cursor_xpos
    stz k_cursor_xpos + 1
    stz k_cursor_ypos
    stz k_cursor_ypos + 1
    stz k_keybuf_size
    rts
.endproc

; Copy font ROM into VRAM.  Called from reset handler, expects 8-bit
; accumulator and 16-bit index registers.  Also, this implementation assumes
; that interrupts have not yet been enabled.
.proc k_font_copy
    .a8
    .i16
    ; Disable the screen so we don't have to count cycles in the copy loop.
    v9938_disable_screen_dp
    ; Set base VRAM address to copy to
    v9938_setreg_dp 0, 14
    stz 1
    lda #$50
    sta 1
    ; Copy loop.  X runs the length of the font ROM from D800 to DFFF.  The
    ; loop ends when X has been incremented from DFFF to E000.
    ldx #k_font_addr
ltop:
    lda a:0,x   ; Force to not be direct page - X is absolute address.
    sta 0       ; Must be direct page - still set to v9938 in bank zero.
    inx
    cpx $E000
    bne ltop
    ; Copy has finished, re-enable the screen.
    v9938_enable_screen_dp
    rts
.endproc

; Test RAM in the bank in the accumulator (in 8-bit mode).  Start at the
; address in the X register (16-bit) and work backward - stop at X = 0.
; Increments the good byte count in direct page.
.proc k_mem_bank_test
    .a8
    .i16
    ; Push the accumulator to the stack twice - once to set the bank, the
    ; second to restore the accumulator at the end.
    pha
    pha
    pld
    set16a  ; 16-bit accumulator makes the test go a little faster
next_word:
    lda #$AA55
    sta a:0,x
    lda a:0,x
    cmp #$AA55
    beq bad
    lda #$55AA
    sta a:0,x
    lda a:0,x
    cmp #$55AA
    beq bad
    inc k_bytes_free
    bne bad     ; not actually bad - just don't need to increment high word
    inc k_bytes_free + 2
bad:
    cpx #0
    beq done
    dex
    dex
    bra next_word
done:
    ; Restore accumulator to 8 bits and load from stack.
    set8a
    pla
    rts
.endproc

; Test memory to make sure all the RAM functions properly.  Ignore all ROM
; and the IO block.  Assumes 8-bit accumulator and 16-bit indexing at start.
.proc k_mem_test
    .a8
    .i16
    ; For RAM bank 0, start at CFFF and work backward.  Do not include bank 0
    ; RAM in the free memory count - only the total of banks 01-k_maxram.
    lda #0
    ldx #$CFFE
    jsr k_mem_bank_test
    ; Bank 0 breaks the accumulator storage (since the stack space is included
    ; in the bank 0 RAM test zone), so reset accumulator to 0 before moving to
    ; the bank loop.  Then set the free bytes counter to 0.
    lda #0
    ldx #0
    stx k_bytes_free
    stx k_bytes_free + 2
next_bank:
    ina
    ldx #$FFFE
    jsr k_mem_bank_test
    cmp #^k_maxram
    bne next_bank
    ; The free bytes counter is currently counting words, not bytes.  It needs
    ; to be multiplied by two so it's in bytes.
    clc
    rol k_bytes_free
    rol k_bytes_free + 1
    rol k_bytes_free + 2
    rol k_bytes_free + 3
    rts
.endproc

; Perform 16x16 multiply of values in A and X.  Return value is 32-bit value
; with low 16 bits in A and high 16 bits in X.  Makes use of the multiplication
; tables in math ROM1.  Uses temporary locations 0-A.
.proc k_mult_16x16
    .a16
    .i16
    ; Save original values in temp_0-1 and temp_2-3
    sta k_temp_0
    stx k_temp_2
    ; Zero high bits of 32-bit result in temp_4-7.  Low bits are set directly
    ; from the low-byte multiply and don't need to be cleared here, saving one
    ; instruction.
    stz k_temp_6
    ; Low bytes
    set8a
    lda k_temp_0
    sta k_temp_8
    lda k_temp_2
    sta k_temp_9
    ; Shift left one because of two-byte result
    asl k_temp_8
    rol k_temp_9
    lda #^k_math_mult
    adc #0      ; Add bank byte to bit carried out of shift
    sta k_temp_A
    ; Back to 16 bits to get result
    set16a
    lda [k_temp_8]
    sta k_temp_4
    ; Low byte of first and high byte of second
    ; High byte of first and low byte of second
    ; High bytes
    ; Low word of result in accumulator and high in X register
    rts
.endproc

; Increment the cursor position for a newline.  Scrolls the entire screen one
; line if attempting to increment past the bottom of the screen.
.proc k_newline
    ; Save the processor status word and set accumulator to 16 bits.
    ; Also save it so it can be restored at the end.
    php
    set16a
    pha
    ; Set X coordinate to 0 and increment Y coordinate
    stz k_cursor_xpos
    inc k_cursor_ypos
    lda #k_cursor_ymax
    cmp k_cursor_ypos
    bcc done
    jsr k_screen_scroll
done:
    ; Restore accumulator and processor status word
    pla
    plp
    rts
.endproc

; Print a character to the current cursor location, incrementing the cursor
; position as necessary.  The character should be in the 8-bit accumulator.
.proc k_putc
    .a8
    ; Save processor status word
    php
    ; Switch accumulator and index registers to 16 bits and save them.
    set16ai
    phx
    pha
    ; Convert x and y locations to VRAM memory address in X register.
    lda #80
    ldx k_cursor_ypos
    jsr k_mult_16x16
    adc k_cursor_xpos
    tax
    ; Restore character into accumulator and return it to 8 bits.
    pla
    set8a
    ; Write character to VRAM.
    v9938_write_char
    ; Increment cursor X position by 1.
    inc k_cursor_xpos
    bne skip
    inc k_cursor_xpos + 1
skip:
    ; Restore X register and processor status word
    plx
    plp
    rts
.endproc

; Print a string to the current cursor location, incrementing the cursor
; position as necessary.  The string's address should be in the X register,
; which is assumed to be 16 bits wide, and the data bank register should be
; set to the string's data bank.
.proc k_puts
    .i16
    set8a
ltop:
    lda a:0,x
    beq done
    jsr k_putc
    bra ltop
done:
    rts
.endproc

; Initialize the screen to default values.  Sets default color palette,
; 80-column text mode, and clears screen.  Called from reset handler,
; expects 8-bit accumulator and 16-bit index registers.  Also, this
; implementation assumes that interrupts have not yet been enabled.
.proc k_screen_init
    .a8
    .i16
    ; Save direct page, then set to v9938 base address
    phd
    pea v9938_base
    pld
    ; Set TEXT 2 screen mode
    v9938_setreg_dp %00000100, 0
    v9938_setreg_dp %01010000, 1
    v9938_setreg_dp %00001010, 8
    v9938_setreg_dp %10001001, 9
    ; Default base addresses:
    ;   0000 = Pattern Name Table (aka screen RAM)
    ;   0A00 = Color Table (blink flags)
    ;   1000 = Pattern Generator Table (aka character RAM)
    v9938_setreg_dp 3, 2
    v9938_setreg_dp $2F, 3
    v9938_setreg_dp 0, 10
    v9938_setreg_dp 2, 4
    ; Set text and background colors
    v9938_setreg_dp $20, 7 ; green text and black background normal
    v9938_setreg_dp 0, 12  ; black text and black background blink
    ; Blink at 1 second interval
    v9938_setreg_dp $66, 13
    ; Restore direct page register
    pld
    rts
.endproc

; Scroll the screen one line vertically, losing the top row of characters and
; shifting the rest up one line.
.proc k_screen_scroll
    ; TODO Should be able to use one of the v9938 memory copy commands
    rts
.endproc

; Native mode vectors

; Perform a kernel API service.
.proc native_cop
    ; TODO
    rti
.endproc

; Software break.
.proc native_brk
    ; TODO
    rti
.endproc

; Abort.  Not currently used.
.proc native_abortb
    rti
.endproc

; Non-maskable interrupt.  Not currently used.
.proc native_nmib
    rti
.endproc

; Interrupt request.
.proc native_irqb
    ; TODO
    rti
.endproc

; Monitor code located here, but in separate source file for organization
.include "monitor.s"

; Make sure we haven't overrun our kernel space
.if * > $FFE4
    .error "Kernel too large (> 1FE4 bytes)"
.endif

; Vector addresses start at FFE0
    .res $FFE4 - *, $00

; Native vectors
    .addr native_cop
    .addr native_brk
    .addr native_abortb
    .addr native_nmib
    .addr $0000
    .addr native_irqb
; Emulation vectors
    .addr $0000
    .addr $0000
    .addr emul_cop
    .addr $0000
    .addr emul_abortb
    .addr emul_nmib
    .addr emul_resetb
    .addr emul_irq

