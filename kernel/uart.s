; DT65PC National Semiconductor PC16550D UART driver.
; Copyright (C) 2023 David Terhune
;

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
; MCR control bits
;======================================================================
uMCR_LOOP    = $10

;======================================================================
; Loopback test of PC16550D UART
; The base address of the UART should be just above the return address
; on the stack. It will be replaced with a 2-byte code indicating
; test success or failure. Zero is success, while non-zero means
; failure.
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
