





.include "m2560def.inc"
.include "myproc.h"
.extern initialize
.extern read
.extern write

.cseg
.org 0

.global initialize:
movw    r30, r24
ld      r24, ZL
clr		ZH
or      r24, r22
st      ZL, r24
ret

