
#ifndef DIGIPORT_H_
#define DIGIPORT_H_
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#define SET_IN_PORT		 (0b00000000)
#define SET_OUT_PORT	 (0b11111111)
#define SET_ACTIVE_HIGH  (0b11111111)
#define SET_ACTIVE_LOW	 (0b00000000)

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

class DigiPort{
	private:
		volatile uint8_t* Portreg; // port adresse
		uint8_t Mode;				// Dont need but its ok for easily understanding (= *Portreg) Input/Output
		static uint8_t PortNumber[11];  // PA - PK 
		//Static variables when used inside function are initialized only once, and then they hold there value even through function calls.
		
		uint8_t ActiveStatus; // Active high/low
	public:
		DigiPort(uint8_t PN,uint8_t M,uint8_t AS);  // Constructor
		void inline write(uint8_t bits);
		void inline toggle(uint8_t bits = 0xFF);
		void inline off(uint8_t bits = 0xFF);
		void inline on(uint8_t bits = 0xFF);
		uint8_t inline read_raw(uint8_t maske = 0xFF);
		uint8_t inline read_busy_wait(uint8_t maske = 0xFF);
};

// Prozeduren
DigiPort::DigiPort(uint8_t PN,uint8_t M,uint8_t AS){
		Mode = M;
		ActiveStatus = AS;
	if(PortNumber[PN-1] == 0){ // CUMA BUAT POSISI AJA karena array mulai dari 0
		switch(PN){
			case PA:
			Portreg = &DDRA;
			break;
			
			case PB:
			Portreg = &DDRB;
			break;
			
			case PC:
			Portreg = &DDRC;
			break;
			
			case PD:
			Portreg = &DDRD;
			break;
			
			case PE:
			Portreg = &DDRE;
			break;
			
			case PF:
			Portreg = &DDRF;
			break;
			
			case PG:
			Portreg = &DDRG;
			break;
			
			case PH:
			Portreg = &DDRH;
			break;
			
			case PJ:
			Portreg = &DDRJ;
			break;

			case PK:
			Portreg = &DDRK;
			break;
		}
		*Portreg = M;		// DDRXvalue set to Input/Output 
		PortNumber[PN-1]++;
	}
		
}

void inline DigiPort::write(uint8_t bits){
	if(ActiveStatus == SET_ACTIVE_HIGH){
		*(Portreg+1) = ~(uint8_t)(bits);	// active high
	}
	else{
		*(Portreg+1) = (uint8_t)(bits);	// active low
	}
}

void inline DigiPort::toggle(uint8_t bits){
	*(Portreg+1) ^= (uint8_t)(bits); // xor
}

void inline DigiPort::on(uint8_t bits){
	if(ActiveStatus == SET_ACTIVE_HIGH){
		*(Portreg+1) |= (uint8_t)(bits);  //active high
	}
	else{
		*(Portreg+1) &= ~(uint8_t)(bits); //active low
	}
}

void inline DigiPort::off(uint8_t bits){
	if(ActiveStatus == SET_ACTIVE_HIGH){
		*(Portreg+1) &= ~(uint8_t)(bits); //active high
	}
	else{
		*(Portreg+1) |= (uint8_t)(bits); // active low
	}
}

uint8_t inline DigiPort::read_raw(uint8_t maske){
	if(ActiveStatus == SET_ACTIVE_HIGH){
		return *(Portreg-1) |= ~(uint8_t)(maske); // active high
	}
	else{
		return *(Portreg-1) &= (uint8_t)(maske); // active low
	}
	
}

uint8_t inline DigiPort::read_busy_wait(uint8_t maske){
	uint8_t temp = read_raw();
	do{
		//_delay_ms(100);
	} while(read_raw()==temp);
	
	return temp;
}

uint8_t DigiPort::PortNumber[11]={0};
#endif /* DIGIPORT_H_ */