; DT65PC Math library
; Copyright (C) 2023 David Terhune

;======================================================================
; Verify math ROM presence and set appropriate k_flag bits.
;======================================================================
.proc m_rom_test
    .a16
    ; Use the SIN function for ROM0. This does not JSR to the real
    ; sin function because that checks for ROM0 presence and the ROM0
    ; flag has not been properly set yet.
    lda #$2000      ; 45 degrees for the sine table
    asl a           ; left-shift to convert to table index
    sta k_temp      ; store in zero-page
    lda #m_sin_tbl  ; base table address
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

    ; Use the MULT function for ROM1. This does not JSR to the real
    ; mul function because that checks for ROM1 presence and the ROM1
    ; flag has not been properly set yet.
    lda #$F008      ; $F0 * $08
    asl a           ; left-shift to convert to table index
    sta k_temp      ; store in zero-page
    lda #m_mul_tbl  ; base table address
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
    rts
.endproc

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
