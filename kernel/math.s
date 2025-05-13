; DT65PC Math library
; Copyright (C) 2023-25 David Terhune

;======================================================================
; Convert binary bytes in scratch to zero-terminated ASCII in a buffer.
; The number is in the scratch buffer and the buffer address is in
; temp. The number of bytes to convert is in the X register.
; Clobbers all 3 registers.
;======================================================================
.proc m_b2a
    php
    set8a

    ; For each byte in the input, convert to hex digits. Start with the
    ; most-significant byte and work down.
    dex
    ldy #0
next_byte:
    lda k_zero::scratch,X
    pha                 ; Save value
    lsr                 ; Shift high nibble to low
    lsr
    lsr
    lsr
    jsr m_xdigit
    sta (k_zero::temp),Y  ; Store to string
    iny
    pla
    and #$0F            ; Mask off high nibble
    jsr m_xdigit
    sta (k_zero::temp),Y
    iny
    dex
    bpl next_byte

    lda #0              ; Zero-terminate the string and return
    sta (k_zero::temp),Y
    plp
    rts
.endproc

;======================================================================
; Convert unsigned 32-bit number to zero-terminated ASCII decimal
; string. Puts result in a buffer supplied by the caller. The number is
; on the stack above the return address, the buffer address above that.
; Clobbers all 3 registers, temp, and the scratchpad.
;======================================================================
.proc m_ul2a
    ; Save processor status and insure 16-bit accumulator.
    php
    set16a

    ; Copy input to temp and convert to BCD in scratch.
    lda 4,S
    sta k_zero::temp
    lda 6,S
    sta k_zero::temp + 2
    jsr m_ultemp2bcd

    ; Put buffer address in temp and call binary to ASCII.
    lda 8,S
    sta k_zero::temp
    ldx #6
    jsr m_b2a

    ; Restore saved processor status and return
    plp
    rts
.endproc

;======================================================================
; Convert unsigned 32-bit number in temp to 6-byte BCD number in
; scratch. A maximum of 10 digits will be populated with non-zero
; values.
; Clobbers the accumulator and Y-register. The accumulator is assumed
; to be in 16-bit mode and the index registers in 8-bit mode.
;======================================================================
.proc m_ultemp2bcd
    .a16

    ; Zero the used scratchpad bytes.
    stz k_zero::scratch
    stz k_zero::scratch + 2
    stz k_zero::scratch + 4

    ; Set decimal mode and initialize for 32-bit input.
    php
    sed
    ldy #32

next_bit:
    ; Shift input left by one, leaving MSB in carry flag.
    asl k_zero::temp
    rol k_zero::temp + 2
    ; Multiply output by 2 by adding it to itself. The carry bit from
    ; the shift is also added, which accumulates the result.
    lda k_zero::scratch
    adc k_zero::scratch
    sta k_zero::scratch
    lda k_zero::scratch + 2
    adc k_zero::scratch + 2
    sta k_zero::scratch + 2
    lda k_zero::scratch + 4
    adc k_zero::scratch + 4
    sta k_zero::scratch + 4
    dey
    bne next_bit

    plp
    rts
.endproc

;======================================================================
; Converts binary digit in the accumulator into its ASCII equivalent.
; The high nibble is assumed to be clear. This method works correctly
; for both hex and decimal digits.
; The accumulator is assumed to be in 8-bit mode.
;======================================================================
.proc m_xdigit
    .a8
    ora #$30
    cmp #'9'
    beq :+
    bcc :+
    adc #6
:   rts
.endproc

;======================================================================
; Verify math ROM presence and set appropriate kernel flag bits.
; Assumes the accumulator is in 16-bit mode.
; Clobbers the accumulator and the temp pointer.
;======================================================================
.proc m_rom_test
    .a16
    ; Use the SIN function for ROM0. This does not JSR to the real
    ; sin function because that checks for ROM0 presence and the ROM0
    ; flag has not been properly set yet.
    lda #$2000          ; 45 degrees for the sine table
    asl a               ; left-shift to convert to table index
    sta k_zero::temp    ; store in zero-page
    lda #m_sin_tbl      ; base table address
    adc #0              ; add one if shift had a carry
    sta k_zero::temp + 2 ; store bank byte
    lda [k_zero::temp]  ; get the answer
    cmp #$5A82
    bne rom0_bad
    set8a
    lda #1          ; set ROM0 present flag
    tsb k_zero::flags
    set16a
rom0_bad:

    ; Use the MULT function for ROM1. This does not JSR to the real
    ; mul function because that checks for ROM1 presence and the ROM1
    ; flag has not been properly set yet.
    lda #$F008          ; $F0 * $08
    asl a               ; left-shift to convert to table index
    sta k_zero::temp    ; store in zero-page
    lda #m_mul_tbl      ; base table address
    adc #0              ; add one if shift had a carry
    sta k_zero::temp + 2 ; store bank byte
    lda [k_zero::temp]  ; get the answer
    cmp #$0780
    bne rom1_bad
    set8a
    lda #2          ; set ROM1 present flag
    tsb k_zero::flags
    set16a
rom1_bad:
    rts
.endproc

;======================================================================
; Table bank values
;======================================================================
m_square_tbl = $E0

m_invert_tbl = $E4

m_sin_tbl = $E8
m_asin_tbl = $EA

m_atan_tbl = $EC

m_log2_tbl = $ED
m_alog2_tbl = $F0

m_log2a_tbl = $F2
m_alog2a_tbl = $F4

m_log2b_tbl = $F6
m_alog2b_tbl = $F8

m_sqrt1_tbl = $FA
m_sqrt2_tbl = $FB
m_sqrt3_tbl = $FC

m_mul_tbl = $FE

math_bitrev14 = $EF0000
math_bitrev13 = $EF8000
math_bitrev12 = $EFC000
math_bitrev11 = $EFE000
math_bitrev10 = $EFF000
math_bitrev09 = $EFF800
math_bitrev08 = $EFFC00
math_bitrev07 = $EFFE00
math_bitrev06 = $EFFF00
math_bitrev05 = $EFFF80
math_bitrev04 = $EFFFC0
math_bitrev03 = $EFFFE0
math_bitrev02 = $EFFFF0
