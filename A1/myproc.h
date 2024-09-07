/*
 * myproc.h
 *
 * Created: 30/11/2021 01:57:33
 * Author : Mee
 */ 

#define INPUT  (0b00000000)
#define OUTPUT (0b11111111)

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>

extern void initialize(volatile uint8_t* portreg,uint8_t mode);
extern void write(volatile uint8_t* portreg,uint8_t bits);
extern uint8_t read(volatile uint8_t* portreg);

extern void initialize(volatile uint8_t* portreg,uint8_t mode){
	if(mode == OUTPUT){
		*portreg = OUTPUT;
		}else if(mode == INPUT) {
		*portreg = INPUT;
		}else{
		printf("Bitte nur 0b00000000 (INPUT) oder 0b11111111 (OUTPUT) in mode schreiben.\n");
	}
}

extern void write(volatile uint8_t* portreg,uint8_t bits){
	*portreg = bits;
}

extern uint8_t read(volatile uint8_t* portreg){
	return *portreg;
}

