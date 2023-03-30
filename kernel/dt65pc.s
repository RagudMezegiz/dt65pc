; DT65PC kernel source code
; Copyright (c) 2019-2023 David Terhune
;
; ca65 assembler syntax version

; Force 65816 mode in case the command-line invocation didn't
.p816

; Kernel ROM starts at E000
    .org $E000

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

;======================================================================
; Kernel API call jump table.  All API calls are to two-byte addresses
; residing in kernel ROM.
;======================================================================

    .res $E100 - *, $00  ; leave all 128 jump table slots free

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

    ; Check for low RAM first, to see if it's safe to set the stack
    ; pointer. Use bit patterns that aren't likely to be random.
    lda #$55
    sta a:0     ; force absolute rather than zero page
    lda a:0
    cmp #$55
    bne post_fail
    lda #$AA
    sta a:0
    lda a:0
    cmp #$AA
    bne post_fail

    ; Low RAM appears to exist, so it should be safe to set the stack
    ; pointer and start using subroutines.
    lda #$FF
    tcs

    ; Switch to native mode
    clc
    xce

    ; TODO Check memory-mapped I/O devices

    ; See if the math ROMs are installed properly.

    ; Use the SIN function for ROM0
    set16a
    lda #$2000      ; 45 degrees for the sine table
    asl a           ; left-shift to convert to table index
    sta 0           ; store in zero-page
    lda #k_math_sin ; base table address
    adc #0          ; add one if shift had a carry
    sta 2           ; store bank byte
    lda [0]         ; get the answer
    cmp #$5A82
    bne post_fail   ; TODO Set NO ROM0 flag instead

    ; Use the MULT function for ROM1
    lda #$F008      ; $F0 * $08
    asl a           ; left-shift to convert to table index
    sta 0           ; store in zero-page
    lda #k_math_mult    ; base table address
    adc #0          ; add one if shift had a carry
    sta 2           ; store bank byte
    lda [0]         ; get the answer
    cmp #$0780
    bne post_fail   ; TODO Set NO ROM1 flag instead

    ; Check high RAM to see how many banks are populated. Save the
    ; bank count in zero page to print later.

    ; End of POST
    jmp mon_start

post_fail:
    ; TODO Figure out how to show POST failure (maybe blink LEDs)
    stp
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
