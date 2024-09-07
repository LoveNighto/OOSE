/*
 * A2_C.c
 *
 * Created: 30/11/2021 11:19:31
 * Author : Mee
 */ 

#include <avr/io.h>
#ifndef F_CPU
	#define F_CPU 16000000UL
#endif
#include <util/delay.h>
#include "DigiPort.h"

DigiPort_t ledport;
DigiPort_t swport;
int main(void)
{
	initialize (&ledport, PA, SET_OUT_PORT);
	initialize (&swport, PK, SET_IN_PORT);
	write (&ledport, 0xAA);
	//_delay_ms(500);
	off (&ledport, 0x0F);
	//_delay_ms(500);
	on (&ledport, 0xF0);
	//_delay_ms(500);
	toggle (&ledport, 0x3C);
	//_delay_ms(500);
	toggle (&ledport, 0x3C);

	read_busy_wait(&swport, 0xFF);
	off(&ledport, 0xFF);
	do {
		write (&ledport, read_raw(&swport,0xFF));
	} while (1);
}

