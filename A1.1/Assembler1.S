/*
.include <avr/io.h>
.include "m2560def.inc"
.include "myproc.h"
*/
#include <avr/io.h>

/*
.cseg
.org 0
*/

.global initialize
.global read
.global write

; https://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_port_pass 

initialize:

movw    r30, r24			; r30 is ZL(30:31)
st      Z, r22
ret

read:

movw    r30, r24		
sbiw	Z, 1					; tabelle
ld      r24, Z
ret

write:

movw    r30, r24	
adiw	Z, 1		
st      Z, r22 
ret
/*
*/