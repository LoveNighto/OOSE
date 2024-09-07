
#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000L
#endif
#include <util/delay.h>
#include "DigiPort.h"


int main(void)
{
	DigiPort ledport(PA, SET_OUT_PORT, SET_ACTIVE_LOW);
	DigiPort swport(PK, SET_IN_PORT, SET_ACTIVE_LOW);

	ledport.write (0xAA);
	_delay_ms(500);

	ledport.off (0x0F);
	_delay_ms(500);

	ledport.on (0xF0);
	_delay_ms(500);
	ledport.toggle (0x3C);
	_delay_ms(500);
	ledport.toggle (0x3C);
	_delay_ms(1000);
	ledport.toggle (0xFF);
	_delay_ms(5000);
	ledport.toggle (0xFF);
	_delay_ms(5000);
	ledport.toggle (0xFF);
	swport.read_busy_wait();
	ledport.off(0xFF);
	do {
		ledport.write (swport.read_raw());
	} while (1);

}