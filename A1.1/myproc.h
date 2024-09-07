/*
 * myproc.h
 *
 * Created: 01/12/2021 11:43:35
 *  Author: Mee
 */ 

#include <avr/io.h>
#include <stdio.h>

#ifndef MYPROC_H_
#define MYPROC_H_

#define OUTPUT (0b11111111)
#define INPUT  (0b00000000)

extern void initialize(volatile uint8_t* portreg,uint8_t mode);
extern void write(volatile uint8_t* portreg,uint8_t bits);
extern uint8_t read(volatile uint8_t* portreg);

#endif /* MYPROC_H_ */



/*
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
*/