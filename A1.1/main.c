/*
 * A1.1.c
 *
 * Created: 01/12/2021 11:39:06
 * Author : Mee
 */ 

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
//#include "Assembler1.S"
#include "myproc.h"

#define INPUT  (0b00000000)
#define OUTPUT (0b11111111)

int main(void)
{
	uint8_t key,led;
	initialize(&DDRK,INPUT); // key
	initialize(&DDRA,OUTPUT); // led				//PIN for Input PORT for OUTPUT
	
	led = 0b11111111; // led am Anfang ausgeschaltet Active LOW
	key = 0b00000000; // keys am Anfang nicht gedruckt Active LOW
	
	write(&DDRK, key); // key = bits
	write(&DDRA, led);
	
	// 3 Auswahl von LEDs-Kombinationen
	while(1){
		key = read(&DDRK);
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
		write(&DDRA, led);
	}
	return 0;
}

