/*
 * DigiPort.h
 *
 * Created: 01/12/2021 14:43:32
 *  Author: Mee
 */ 


#ifndef DIGIPORT_H_
#define DIGIPORT_H_
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#define SET_IN_PORT  (0b00000000)
#define SET_OUT_PORT  (0b11111111)

#define PA ((uint8_t)(1))
#define PB ((uint8_t)(2))
#define PC ((uint8_t)(3))
#define PD ((uint8_t)(4))
#define PE ((uint8_t)(5))
#define PF ((uint8_t)(6))
#define PG ((uint8_t)(7))
#define PH ((uint8_t)(8))
//#define PI ((uint8_t)(9))
#define PJ ((uint8_t)(9))
#define PK ((uint8_t)(10))


typedef struct{

	volatile uint8_t* portreg;
	uint8_t mode;
	uint8_t status;
	
} DigiPort_t;

// Declinierung
void initialize(DigiPort_t* port, uint8_t port_nummer, uint8_t mode);
void write(DigiPort_t* port, uint8_t bits);
void toggle(DigiPort_t* port, uint8_t bits);
void off(DigiPort_t* port,uint8_t bits);
uint8_t read_raw(DigiPort_t* port, uint8_t maske);
uint8_t read_busy_wait(DigiPort_t* port, uint8_t maske);


// Prozeduren BITTE NICHT BENUTZEN ( NICHT AKTUELL AKTUALISIERT )
void initialize(DigiPort_t* port, uint8_t port_nummer, uint8_t mode){
		switch(port_nummer){
			case PA:
			port->portreg = &DDRA;		// portreg pointed to address ddra		
			port->status = PA;
			break;
			
			case PB:
			port->portreg = &DDRB;
			port->status = PB;
			break;
			
			case PC:
			port->portreg = &DDRC;
			port->status = PC;
			break;
			
			case PD:
			port->portreg = &DDRD;
			port->status = PD;
			break;
			
			case PE:
			port->portreg = &DDRE;
			port->status = PE;
			break;
			
			case PF:
			port->portreg = &DDRF;
			port->status = PF;
			break;
			
			case PG:
			port->portreg = &DDRG;
			port->status = PG;
			break;
			
			case PH:
			port->portreg = &DDRH;
			port->status = PH;
			break;
			
			case PJ:
			port->portreg = &DDRJ;
			port->status = PJ;
			break;
			
			case PK:
			port->portreg = &DDRK;
			port->status = PK;
			break;
		}
		*port->portreg = mode;  //
		port->mode = mode; //
}

void write(DigiPort_t* port, uint8_t bits){
	if(port->mode==SET_OUT_PORT){
		*((port->portreg)+1)=bits;	// active high
		// *((port->portreg)+1) = ~(uint8_t)(bits);	// active low
	}
	else{
		printf("FAILED: This port is for writing, please use input() \n");
	}
}

void toggle(DigiPort_t* port, uint8_t bits){
	if(bits == (uint8_t)(0x00)){		//set default value
		bits = (uint8_t)(0xFF);
	}
	
	if(port->mode==SET_OUT_PORT){
		*((port->portreg)+1) ^= (uint8_t)(bits); // xor
	}
	
	else{
		printf("FAILED: This port is for normally for output! \n");
	}
}

void on(DigiPort_t* port, uint8_t bits){
	if(port->mode==SET_OUT_PORT){
		//*((port->portreg)+1) &= ~(uint8_t)(bits); //active low
		*((port->portreg)+1) |= (uint8_t)(bits);  //active high
	}
	else{
		printf("FAILED: This port is for normally for output! \n");
	}
}

void off(DigiPort_t* port, uint8_t bits){
	if(port->mode==SET_OUT_PORT){
		//*((port->portreg)+1) |= (uint8_t)(bits); // active low
		*((port->portreg)+1) &= ~(uint8_t)(bits); //active high
	}
	else{
		printf("FAILED: This port is for normally for output! \n");
	}
}

uint8_t read_raw(DigiPort_t* port, uint8_t maske){
	if(maske== (uint8_t)(0x00)){	// default value 
		maske= (uint8_t)(0xFF);
	}
	return *((port->portreg)-1) &= (uint8_t)(maske); // active high
	//return *((port->portreg)-1) |= ~(uint8_t)(maske); // active low
}

uint8_t read_busy_wait(DigiPort_t* port, uint8_t maske){
	uint8_t temp = read_raw(port, maske);
	do{
		//_delay_ms(100);
	} while(read_raw(port, maske)==temp);
	
	return read_raw(port, maske);
}

#endif /* DIGIPORT_H_ */