; DT65PC kernel source code
; Copyright (c) 2019-2025 David Terhune
;
; ca65 assembler syntax version

; Force 65816 mode in case the command-line invocation didn't
.p816

; Make sure we can use C-style string escapes
.feature string_escapes

; Kernel ROM starts at C000
    .org $C000

; Macro includes
.include "macros.s"

;======================================================================
; Symbolic constants
;======================================================================

; Kernel zero-page storage
.struct k_zero
    banks .byte         ; Number of high RAM banks present
    flags .byte         ; Bit 0 = math ROM0 present
                        ; bit 1 = math ROM1 present
    temp  .dword        ; 4-byte temporary accumulator
    scratch .res 16     ; 16-byte scratchpad
.endstruct

; Kernel storage blocks. Each is one page in length.
k_base = $0400
k_buf1 = $0500
k_buf2 = $0600

;======================================================================
; Kernel API call jump table.  All API calls are to two-byte addresses
; residing in kernel ROM.
;======================================================================

    .res $C100 - *, $00  ; leave all 128 jump table slots free

;======================================================================
; Kernel string constants
;======================================================================

; Ready message printed to terminal or screen on startup.
; It will be followed by the free byte count from high RAM.
msg_ready:
    .asciiz "DT65PC ready. "
msg_bytes_free:
    .asciiz " bytes free.\r\n"

;======================================================================
; Library includes
;======================================================================
.include "math.s"
.include "uart.s"
.include "monitor.s"

uart_make TERM, $B000
uart_make SERIAL, $B100

;======================================================================
; Unused vectors. All unused vectors currently throw a fatal error and
; halt the system.
;======================================================================

emul_cop:
emul_abortb:
emul_nmib:
emul_irq:
native_abortb:
native_nmib:
    ; TODO Enter monitor instead of fatal stop?
    stp

;======================================================================
; Reset handler
;======================================================================
.proc emul_resetb
    .a8
    .i8
    ; Start of POST

    ; Given that this code is executing, it's probably safe to assume
    ; that the kernel ROM exists.

    ; Switch to native mode and 16-bit accumulator.
    clc
    xce
    set16a

    ; Check for low RAM first, to see if it's safe to set the stack
    ; pointer. Use bit patterns that aren't likely to be random.
    lda #$AA55
    sta a:0     ; force absolute rather than zero page
    lda a:0
    cmp #$AA55
    beq :+
    jmp post_fail
:   lda #$55AA
    sta a:0
    lda a:0
    cmp #$55AA
    beq :+
    jmp post_fail
:

    ; Low RAM appears to exist, so it should be safe to set the stack
    ; pointer and start using subroutines. Stack starts just under the
    ; base address for kernal storage.
    lda #k_base - 1
    tcs

    ; Zero out kernel zero-page storage.
    ldx #.sizeof(k_zero)
next_byte:
    dex
    stz 0,x
    bne next_byte

    ; Check memory-mapped I/O devices.

    ; Check UART0, which is expected to be connected to a terminal so
    ; later tests can print results.
    pea TERM_addr
    jsr u_loop_test
    pla
    bne post_fail

    ; Set up UART0 for terminal I/O
    ; At 4.9152 MHz clock, 19200 baud is divisor 16
    ldy #uLCR_DLAB
    sty TERM::LCR
    ldy #16
    sty TERM::DLL
    ldy #0
    sty TERM::DLM
    ldy #uLCR_8BITS | uLCR_1STOP
    sty TERM::LCR
    ldy #uFCR_FIFO | uFCR_4CHAR
    sty TERM::FCR
    ldy #uMCR_DTR
    sty TERM::MCR

    ; Check UART1
    pea SERIAL_addr
    jsr u_loop_test
    pla
    bne post_fail

    ; TODO Check additional I/O devices here.

    ; High RAM check to save how many banks are populated.
    jsr hiram_test

    pea msg_ready
    jsr TERM_writes
    pla

    ; Print available RAM in bytes, excluding bank 0.
    pea k_buf1
    pei (k_zero::banks) ; Flags is still zero, so this is safe.
    pea 0
    jsr m_ul2a
    pla
    pla
    pla

    ; Increment the buffer address past all the leading zeros before
    ; writing to the terminal.
    ldy #$FF
:   iny
    ldx k_buf1,Y
    cpx #'0'
    beq :-
    sty k_zero::temp
    stz k_zero::temp + 1
    clc
    adc k_zero::temp
    pha
    jsr TERM_writes
    pla
    
    pea msg_bytes_free
    jsr TERM_writes
    pla

    ; See if the math ROMs are installed properly.
    jsr m_rom_test
    lda k_zero::flags
    and #3
    cmp #3
    bne post_fail

    ldy k_zero::flags   ; put flags into Y for sim display

    ; End of POST
    jmp mon_start

post_fail:
    ; TODO Figure out how to show POST failure (maybe blink LEDs)
    stp
.endproc

;======================================================================
; High RAM test
;======================================================================
.proc hiram_test
    .a16
    ldx #1
next_bank:
    phx
    plb
    lda #$AA55
    sta a:0
    lda a:0
    cmp #$AA55
    bne end_banks
    lda #$55AA
    sta a:0
    lda a:0
    cmp #$55AA
    bne end_banks
    inx
    bra next_bank
end_banks:
    dex         ; X holds one more bank than exists, so decrement
    stx k_zero::banks

    ; Reset to bank 0 - Y is still zero after UART tests
    phy
    plb
    rts
.endproc

;======================================================================
; Native mode vectors
;======================================================================

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

; Interrupt request.
.proc native_irqb
    ; TODO
    rti
.endproc

; Make sure we haven't overrun our kernel space
.if * > $FFE4
    .error "Kernel too large (> 3FE4 bytes)"
.endif

; Vector addresses start at FFE4
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
