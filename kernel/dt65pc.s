; DT65PC kernel source code
; Copyright (c) 2019-2023 David Terhune
;
; ca65 assembler syntax version

; Force 65816 mode in case the command-line invocation didn't
.p816

; Kernel ROM starts at C000
    .org $C000

; Macro includes
.include "macros.s"

;======================================================================
; Symbolic constants
;======================================================================

; Math ROM bank addresses
k_math_square   = $E0
k_math_invert   = $E4
k_math_sin      = $E8
k_math_asin     = $EA
k_math_atan     = $EC
k_math_log2     = $ED
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
k_math_alog2    = $F0
k_math_log2a    = $F2
k_math_alog2a   = $F4
k_math_log2b    = $F6
k_math_alog2b   = $F8
k_math_sqrt1    = $FA
k_math_sqrt2    = $FB
k_math_sqrt3    = $FC
k_math_mult     = $FE

; Kernel zero-page storage
k_flags = $00   ; Bit 0 = math ROM0 present
                ; Bit 1 = math ROM1 present
                ; Bit 2 = unused
                ; Bit 3 = unused
                ; Bit 4 = unused
                ; Bit 5 = unused
                ; Bit 6 = unused
                ; Bit 7 = unused

k_temp  = k_flags + 1   ; 4 bytes

k_zero_size = k_temp + 4    ; Number of zero-page bytes used by the kernel

; Kernel storage addresses
k_base      = $0700
bank_count  = k_base    ; 1 byte

; UART base addresses
UART0_BASE  = $B000
UART1_BASE  = $B100

; UART register offsets
uart_RBR = 0
uart_THR = 0
uart_IER = 1
uart_IIR = 2
uart_FCR = 2
uart_LCR = 3
uart_MCR = 4
uart_LSR = 5
uart_MSR = 6
uart_SCR = 7
uart_DLL = 0
uart_DLM = 1

; UART control bits
uartMCR_LOOP    = $10

;======================================================================
; Kernel API call jump table.  All API calls are to two-byte addresses
; residing in kernel ROM.
;======================================================================

    .res $C100 - *, $00  ; leave all 128 jump table slots free

;======================================================================
; Kernel string constant section.
;======================================================================

; Welcome message printed to terminal or screen on startup.
msg_welcome:
    .asciiz "* * * Welcome to the DT65PC! * * *"

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
    bne post_fail
    lda #$55AA
    sta a:0
    lda a:0
    cmp #$55AA
    bne post_fail

    ; Low RAM appears to exist, so it should be safe to set the stack
    ; pointer and start using subroutines. Stack starts just under the
    ; base address for kernal storage.
    lda #k_base - 1
    tcs

    ; Zero out kernel zero-page storage.
    ldx #k_zero_size
next_byte:
    dex
    stz 0,x
    bne next_byte

    ; Check memory-mapped I/O devices.

    ; Check UART0, which is expected to be connected to a terminal so later
    ; tests can print results.
    pea UART0_BASE
    jsr uart_loop_test
    pla
    bne post_fail

    ; TODO Set up UART0 for terminal I/O

    ; Check UART1
    pea UART1_BASE
    jsr uart_loop_test
    pla
    bne post_fail

    ; High RAM check to save how many banks are populated.
    jsr hiram_test

    ; TODO Print available RAM

    ; See if the math ROMs are installed properly.

    ; Use the SIN function for ROM0
    lda #$2000      ; 45 degrees for the sine table
    asl a           ; left-shift to convert to table index
    sta k_temp      ; store in zero-page
    lda #k_math_sin ; base table address
    adc #0          ; add one if shift had a carry
    sta k_temp + 2  ; store bank byte
    lda [k_temp]    ; get the answer
    cmp #$5A82
    bne rom0_bad
    set8a
    lda #1          ; set ROM0 present flag
    tsb k_flags
    set16a
rom0_bad:

    ; Use the MULT function for ROM1
    lda #$F008      ; $F0 * $08
    asl a           ; left-shift to convert to table index
    sta k_temp      ; store in zero-page
    lda #k_math_mult    ; base table address
    adc #0          ; add one if shift had a carry
    sta k_temp + 2  ; store bank byte
    lda [k_temp]    ; get the answer
    cmp #$0780
    bne rom1_bad
    set8a
    lda #2          ; set ROM1 present flag
    tsb k_flags
    set16a
rom1_bad:

    ldy k_flags     ; put flags into Y for sim display

    ; End of POST
    jmp mon_start

post_fail:
    ; TODO Figure out how to show POST failure (maybe blink LEDs)
    stp
.endproc

;======================================================================
; Loopback test of PC16550D UART
; The base address of the UART should be just above the return address
; on the stack. It will be replaced with a 2-byte code indicating
; test success or failure. Zero is success, while non-zero means
; failure.
;======================================================================
.proc uart_loop_test
    ; Set the loopback bit in the MCR, write a couple bytes to the
    ; transmit register and verify they read back correctly.
    set8a
    lda #uartMCR_LOOP
    ldy #uart_MCR
    sta (3,s),y
    lda #$55
    ldy #uart_THR
    sta (3,s),y
    ; It's supposed to be immediate, but give it a couple cycles before reading
    nop
    lda (3,s),y ; RBR and THR are same address, so no need to change Y
    cmp #$55
    bne test_fail
    lda #$AA
    sta (3,s),y
    nop
    lda (3,s),y
    cmp #$AA
    bne test_fail

    ; Successful test - replace address with 2-byte zero
    set16a
    lda #0
    sta 3,s

; If branch to here, UART address is non-zero and should indicate failure.
; If fall-through, the zeros have already overwritten it.
test_fail:
    set16a
    rts
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
    stx bank_count

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

; Monitor code located here, but in separate source file for organization
.include "monitor.s"

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
