; DT65PC National Semiconductor PC16550D UART driver.
; Copyright (C) 2023 David Terhune
;

;======================================================================
; Macro declaring a UART in struct form
;======================================================================
.macro uart_make name, base
    .struct name
        .org base
        .union
            RBR .byte
            THR .byte
            DLL .byte
        .endunion
        .union
            IER .byte
            DLM .byte
        .endunion
        .union
            IIR .byte
            FCR .byte
        .endunion
        LCR .byte
        MCR .byte
        LSR .byte
        MSR .byte
        SCR .byte
    .endstruct

    ; Address constant
    .ident(.concat(.string(name), "_addr")) = base

    ; Write a string to this UART. The string should be just above the
    ; return address on the stack. Clobbers the accumulator and the Y
    ; register.
    .proc .ident(.concat(.string(name), "_writes"))
        set8a

        ldy #0
    next:
        ; Wait for transmit holding register to be empty.
        lda .ident(.string(name))::LSR
        and #uLSR_THRE
        beq next
        ; Get the next character and exit if finished
        lda (3,s),y
        beq done
        ; Write the character and get the next
        sta .ident(.string(name))::THR
        iny
        bra next

    done:
        ; Restore accumulator and return.
        set16a
        rts
    .endproc
.endmac

;======================================================================
; Register offsets
;======================================================================
uRBR = 0
uTHR = 0
uIER = 1
uIIR = 2
uFCR = 2
uLCR = 3
uMCR = 4
uLSR = 5
uMSR = 6
uSCR = 7
uDLL = 0
uDLM = 1

;======================================================================
; FCR control bits
;======================================================================
uFCR_FIFO   = 1
uFCR_CLRF   = 2
uFCR_CLTF   = 4
uFCR_DMA    = 8
uFCR_1CHAR  = 0
uFCR_4CHAR  = $40
uFCR_8CHAR  = $80
uFCR_14CHAR = $C0

;======================================================================
; LCR control bits
;======================================================================
uLCR_5BITS  = 0
uLCR_6BITS  = 1
uLCR_7BITS  = 2
uLCR_8BITS  = 3
uLCR_1STOP  = 0
uLCR_2STOP  = 4
uLCR_ODDP   = 8
uLCR_EVNP   = $18
uLCR_DLAB   = $80

;======================================================================
; MCR control bits
;======================================================================
uMCR_DTR    = 1
uMCR_RTS    = 2
uMCR_OUT1   = 4
uMCR_OUT2   = 8
uMCR_LOOP   = $10

;======================================================================
; LSR status bits
;======================================================================
uLSR_DR     = 1
uLSR_OE     = 2
uLSR_PE     = 4
uLSR_FE     = 8
uLSR_BI     = $10
uLSR_THRE   = $20
uLSR_TEMT   = $40
uLSR_FIFO   = $80

;======================================================================
; Loopback test of PC16550D UART
; The base address of the UART should be just above the return address
; on the stack. It will be replaced with a 2-byte code indicating
; test success or failure. Zero is success, while non-zero means
; failure. Clobbers the accumulator and the Y register.
;======================================================================
.proc u_loop_test
    ; Set the loopback bit in the MCR, write a couple bytes to the
    ; transmit register and verify they read back correctly.
    set8a
    lda #uMCR_LOOP
    ldy #uMCR
    sta (3,s),y
    lda #$55
    ldy #uTHR
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
; If fall-through, the zeros have already overwritten it. The extra set16a
; will have no effect on fall-through, but it's required on branch.
test_fail:
    set16a
    rts
.endproc
