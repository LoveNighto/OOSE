/*
 * A1Cpp.cpp
 *
 * Created: 30/11/2021 00:43:41
 * Author : Mee
 */ 

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
//#include "Assembler1.S"
#include "myproc.h"

#define OUTPUT (0b11111111)
#define INPUT  (0b00000000)


int main(void)
{
	uint8_t key,led;
	initialize(&DDRK,INPUT); // key
	initialize(&DDRA,OUTPUT); // led				//PIN for Input PORT for OUTPUT
	// DDRK = INPUT;
	// DDRA = OUTPUT;
	
	led = 0b11111111; // led am Anfang ausgeschaltet
	key = 0b00000000; // keys am Anfang nicht gedruckt
	
	write(&PINK,key);
	write(&PORTA, led);
		
   // 3 Auswahl von LEDs-Kombinationen
   while(1){
	   key = read(&PINK);
	    switch(key){
		    case 0b11111110:
		    led=0;
		    break;
		    case 0b11111101:
		    led=0b11110000;
		    break;
		    case 0b11111011:
		    led=0b01010101;
		    break;
	    }
	    // Die Auswahl von oben wird in OUTPUT bsw. PORTB ausgegeben
	    write(&PORTA, led);
   }
   return 0;
}

