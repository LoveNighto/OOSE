;
; ASM_Sample.asm
;
; Created: 01/12/2021 11:10:08
; Author : Mee
;


; Replace with your application code
.include "m2560def.inc"
.cseg
.org 0
init:

;---------------------------- INITIALIZE PORTS --------------------------------
 ldi r16, 0b11111111 ;load register r16 with 1's
 out DDRB, r16 ;configure PORT B as output
 ldi r16, 0b00000000 ;load register r16 with 0's
 out DDRD, r16 ;--> configure PORTD as input
 ldi r16, 0b11111111 ;all LEDs off
 out PORTB, r16
;------------------------------------------------------------------------------
main:
 in r18, PIND ; read PIND buffer
 cpi r18, 0b11111111 ; any switch pressed?
 brne check_sw0 ; if one or more are pressed find out which one
 rjmp main ; ... otherwise jump to main and read again

check_sw0:
 cpi r18, 0b11111110 ; sw0 pressed?
 brne check_sw1 ; no - check next
 ldi r19, 0b00000000 ; load pattern ALL on
 rjmp show_leds
check_sw1:
 cpi r18, 0b11111101 ; sw1 pressed?
 brne check_sw2 ; no - check next
 ldi r19, 0b11110000 ; load pattern right on, left off
 rjmp show_leds
check_sw2:
 cpi r18, 0b11111011 ; sw2 pressed?
 brne main ; none sw was pressed -> read again
 ldi r19, 0b01010101 ; load pattern on/off/on/off ....

show_leds:
 out PORTB, r19 ;send new pattern to LEDs!
 rjmp main