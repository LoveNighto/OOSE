/*
 * HelloWorld.cpp
 *
 * Created: 29/11/2021 23:07:29
 * Author : Mee
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>

#define OUTPUT (0b11111111)
#define INPUT (~OUTPUT)

// Die Zuweisung an ein Port-Register entspricht dem ASM
//DDRB = OUTPUT;

// Befehl OUT
//DDRD = INPUT;
 class BikinKelas {
	 private:
		uint8_t* v_pointer_private;
		static uint8_t imatrix_private[];
	 public:
		BikinKelas BikinConstructor(uint8_t* vpp, uint8_t isi1, uint8_t isi2);
		uint8_t v_public;
		
};

typedef struct {
	uint8_t* pointer1;				// 
	uint8_t de_thing;
	
}BikinStruct;

BikinStruct Struct_contoh1;





int main(void)
{
    while (1) 
    {
	/* Aus Mikrocontroller.net
    // Setzt PortA auf 0x03, Bit 0 und 1 "high", restliche "low":
    PORTB = 0x03;   

    // Setzen der Bits 0,1,2,3 und 4
    // Binär 00011111 = Hexadezimal 1F
    DDRB = 0x1F;    // direkte Zuweisung - unübersichtlich 

    // Ausführliche Schreibweise: identische Funktionalität, mehr Tipparbeit
    // aber übersichtlicher und selbsterklärend: 
    DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4);
	*/
	
	int8_t var1 = 100;
	
	
    }
}

