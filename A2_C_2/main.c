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
DigiPort_h led_h = &ledport;
DigiPort_h sw_h = &swport;

int main(void)
{
	initialize (led_h, PA, SET_OUT_PORT);
	initialize (sw_h, PK, SET_IN_PORT);
	write (led_h, 0xAA);
	_delay_ms(500);
	off (led_h, 0x0F);
	_delay_ms(500);
	on (led_h, 0xF0);
	_delay_ms(500);
	toggle (led_h, 0x3C);
	 _delay_ms(500);
	toggle (led_h, 0x3C);

	read_busy_wait(sw_h, 0xFF);
	off(led_h, 0xFF);

	do {
		write (led_h, read_raw(sw_h,0xFF));
	} while (1);
} 

