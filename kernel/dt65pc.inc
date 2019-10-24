; DT65PC system include
; Copyright (c) 2019 David Terhune
;
; System includes can be used in any program to define kernel services.

; Processor flag definitions.  Bitwise OR the flags together for readable
; REP and SEP instructions.
n_flag = $80
v_flag = $40
m_flag = $20
x_flag = $10
d_flag = 8
i_flag = 4
z_flag = 2
c_flag = 1

; Macros to set 16 or 8-bit modes
.macro set16a
    rep #m_flag
    .a16
.endmac
.macro set16i
    rep #x_flag
    .i16
.endmac
.macro set16ai
    rep #m_flag | x_flag
    .a16
    .i16
.endmac
.macro set8a
    sep #m_flag
    .a8
.endmac
.macro set8i
    sep #i_flag
    .i8
.endmac
.macro set8ai
    sep #m_flag | i_flag
    .a8
    .i8
.endmac


