/*
 * A4.cpp
 *
 * Created: 06/12/2021 12:45:42
 * Author : Mee
 */ 


// TODO: Test 1 Ampel -> 1 Ampel w/ Display -> 3 Ampel -> 3 Amp w/ Disp

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "Basics.h"
#include "DigiPort.h"
#include "LCD.h"
#include "Timer.h"

#define OFF (uint8_t)(0)
#define ON  (uint8_t)(1)

DigiPortRaw keys(PA, SET_IN_PORT);					
DigiPortRaw leds(PB, SET_OUT_PORT);
DigiPortRaw leds2(PC, SET_OUT_PORT);
DigiPortRaw leds3(PE, SET_OUT_PORT);
LCD display(PK, LCD_Type_24x2);
static const char init_top[8] PROGMEM = {"State: "}; // 1,2,3,...29
static const char init_bottom[8] PROGMEM = {"Phase: "}; // startup/normal/finish 
	
static const char disp_normal[7] PROGMEM = {"NORMAL"}; 
static const char disp_start[7] PROGMEM = {"START "}; 
static const char disp_end[7] PROGMEM = {"END   "}; 

void next_state();
//void next_state_two();
//void next_state_three();

Timer16 ticker(TC1, next_state);
//Timer16 ticker2(TC2, next_state_two);
//Timer16 ticker3(TC3, next_state_three);

	
// Klasse Ampel
class Ampel{
	private:
		// als Klassenkonstante der Klasse Ampel, Vektor von Structures
		typedef struct{
			uint8_t led_pattern;
			uint8_t duration;
			uint8_t next;
		} state_t;
		static const state_t state_table[28];
		uint8_t delay; // == duration
		uint8_t current_op_number;
		bool shut_down_signal;
		uint8_t status;
		uint8_t display_column;
		DigiPortRaw* led;
		
		enum state_enum{ustate, none};
		state_enum update =  none;
	public:
		Ampel(uint8_t stat, uint8_t disp_col, DigiPortRaw* portreg){
			status = stat; 
			display_column = disp_col;
			led = portreg;
			};
		inline void next_state();  // so there are 2 Procedure: global and here. both have same name next_state();
		inline void start_up(); 
		inline void shut_down_request(); 
		inline void display_update();
};

// Ampel Tabelle-Index
const Ampel::state_t Ampel::state_table[28] = {
	// Normal OP = loop
	{0b00100001, 5, 1}, // #0
	{0b00100010, 1, 2},
	{0b00110010, 1, 3},
	{0b00110100, 1, 4}, // entry from startup
	{0b00001100, 5, 5},
	{0b00010100, 1, 6}, // exit to shutdown // check sw input
	{0b00010110, 1, 7},
	{0b0100110, 1, 0}, // #7
	// Startup
	{0b01010010, 1, 9}, // #8
	{0b01000000, 1, 10},
	{0b01010010, 1, 11},
	{0b01000000, 1, 12},
	{0b01010010, 1, 13},
	{0b01000000, 1, 14},
	{0b01010010, 1, 15},
	{0b01000000, 1, 16},
	{0b01010010, 1, 17},
	{0b01100100, 5, 3}, // #17
	// Shutdown
	{0b10100100, 5, 19}, // #18
	{0b10010010, 1, 20},
	{0b10000000, 1, 21},
	{0b10010010, 1, 22},
	{0b10000000, 1, 23},
	{0b10010010, 1, 24},
	{0b10000000, 1, 25},
	{0b10010010, 1, 26},
	{0b10000000, 1, 27},
	{0b10010010, 1, 28}
};

// Ampel Objekt erzeugen:
Ampel dieAmpel(OFF, 6, &leds);
Ampel dieAmpel2(OFF, 13, &leds2);
Ampel dieAmpel3(OFF, 19, &leds3);

inline void Ampel::start_up(){
	// Startup from op 8 until loop main
	status = ON;
	current_op_number = 8;		
	delay = state_table[current_op_number].duration;	
	update = ustate;
}

inline void Ampel::next_state(){
	if (status == ON){
		delay--;
		if (shut_down_signal == 1){
			if (current_op_number == 5){				// jump from loop to shutdown only at op 5
				current_op_number = 18;					// Shutdown OP
				delay = state_table[current_op_number].duration;
				led->write(state_table[current_op_number].led_pattern);
			}
		}
		update = ustate;
		if (delay == 0){
			if(current_op_number == 27){
				status = OFF;
				shut_down_signal = 0;
				led->off();
				update = ustate;
			}else{
				current_op_number = state_table[current_op_number].next;
				delay = state_table[current_op_number].duration;
				led->write(state_table[current_op_number].led_pattern);
				update = ustate;
			}
		}
	}
}

inline void Ampel::shut_down_request(){
	shut_down_signal = 1;
}

void next_state(){ // jede 1s
	dieAmpel.next_state();
	dieAmpel2.next_state();
	dieAmpel3.next_state();
};
/*
void next_state_two(){ // jede 1s
	dieAmpel2.next_state();
};
void next_state_three(){ // jede 1s
	dieAmpel3.next_state();
};
*/


inline void Ampel::display_update(){

	switch(update){
		case none:
		break;
		
		case ustate:
		display.set_pos(0, display_column);
		display.write_number(current_op_number, 2);
		
		if(current_op_number <= 7){
			display.set_pos(1, display_column);
			display.write_FLASH_text(disp_normal, 7);
		}else if(current_op_number >= 18){
			display.set_pos(1, display_column);
			display.write_FLASH_text(disp_end, 7);
		}else{
			display.set_pos(1, display_column);
			display.write_FLASH_text(disp_start, 6);
		}
	}
	update = none;
}


int main(void)
{
	sei();
	ticker.start(1000);
	//ticker2.start(1000);
	//ticker3.start(1000);

	display.clear();
	display.set_pos(0,0);
	display.write_FLASH_text(init_top, 7);
	display.set_pos(1, 0);
	display.write_FLASH_text(init_bottom, 7);
	
	leds.off();
	leds2.off();
	leds3.off();
	while(1){
		uint8_t key_val = keys.read_raw();
		switch ((uint8_t)~(key_val)){
			case 0b11111110:
			dieAmpel.start_up();
			break;
			case 0b11111101:
			dieAmpel2.start_up();
			break;
			case 0b11111011:
			dieAmpel3.start_up();
			break;
			case 0b11110111:
			dieAmpel.shut_down_request();
			break;
			case 0b11101111:
			dieAmpel2.shut_down_request();
			break;
			case 0b11011111:
			dieAmpel3.shut_down_request();
			break;

		}
		dieAmpel.display_update();
		/*
		dieAmpel2.display_update();
		dieAmpel3.display_update();
		*/

	}		
}

	/*
	for(int i=0; i<=27; i++){
		ampel_farben_init(&MyAmpel[i]);
	}
	*/
// void ampel_farben_init(Ampel_t* Ampel){
/*
	// Normal OP
	Ampel[0] = {(uint8_t)(00100001), 5, 1}; 
	Ampel[1] = {(uint8_t)(00100010), 1, 2};
	Ampel[2] = {(uint8_t)(00110010), 1, 3};
	Ampel[3] = {(uint8_t)(00110100), 1, 4}; // entry from startup
	Ampel[4] = {(uint8_t)(00001100), 5, 5};
	Ampel[5] = {(uint8_t)(00010100), 1, 6}; // exit to shutdown // check sw input
	Ampel[6] = {(uint8_t)(00010110), 1, 7};
	Ampel[7] = {(uint8_t)(00100110), 1, 0};
	// Startup
	Ampel[8]  = {(uint8_t)(01010010), 1, 9};  
	Ampel[9]  = {(uint8_t)(01000000), 1, 10}; 
	Ampel[10] = {(uint8_t)(01010010), 1, 11};
	Ampel[11] = {(uint8_t)(01000000), 1, 12};
	Ampel[12] = {(uint8_t)(01010010), 1, 13};
	Ampel[13] = {(uint8_t)(01000000), 1, 14};
	Ampel[14] = {(uint8_t)(01010010), 1, 15};
	Ampel[15] = {(uint8_t)(01000000), 1, 16};
	Ampel[16] = {(uint8_t)(01010010), 1, 17};
	Ampel[17] = {(uint8_t)(01100100), 5, 3};
	// Shutdown
	Ampel[18] = {(uint8_t)(10100100), 5, 19};
	Ampel[19] = {(uint8_t)(10100100), 1, 20};
	Ampel[20] = {(uint8_t)(10100100), 1, 21};
	Ampel[21] = {(uint8_t)(10100100), 1, 22};
	Ampel[22] = {(uint8_t)(10100100), 1, 23};
	Ampel[23] = {(uint8_t)(10100100), 1, 24};
	Ampel[24] = {(uint8_t)(10100100), 1, 25};
	Ampel[25] = {(uint8_t)(10100100), 1, 26};
	Ampel[26] = {(uint8_t)(10100100), 1, 27};
	Ampel[27] = {(uint8_t)(10100100), 1, 8};
}
*/
/*
inline void Ampel::start_up(){
	uint8_t end_op = 17;

	for(uint8_t op_number = 8; op_number <= end_op; op_number++){
		delay = state_table[op_number].duration;
		while(delay){
			leds.write(state_table[op_number].led_pattern);
		}
	}
}


{
	// Normal OP
	{(uint8_t)(00100001), 5, 1}, // #0
	{(uint8_t)(00100010), 1, 2},
	{(uint8_t)(00110010), 1, 3},
	{(uint8_t)(00110100), 1, 4}, // entry from startup
	{(uint8_t)(00001100), 5, 5},
	{(uint8_t)(00010100), 1, 6}, // exit to shutdown // check sw input
	{(uint8_t)(00010110), 1, 7},
	{(uint8_t)(00100110), 1, 0},
	// Startup
	{(uint8_t)(01010010), 1, 9}, // #8
	{(uint8_t)(01000000), 1, 10},
	{(uint8_t)(01010010), 1, 11},
	{(uint8_t)(01000000), 1, 12},
	{(uint8_t)(01010010), 1, 13},
	{(uint8_t)(01000000), 1, 14},
	{(uint8_t)(01010010), 1, 15},
	{(uint8_t)(01000000), 1, 16},
	{(uint8_t)(01010010), 1, 17},
	{(uint8_t)(01100100), 5, 3}, // #17
	// Shutdown
	{(uint8_t)(10100100), 5, 19}, // #18
	{(uint8_t)(10100100), 1, 20},
	{(uint8_t)(10100100), 1, 21},
	{(uint8_t)(10100100), 1, 22},
	{(uint8_t)(10100100), 1, 23},
	{(uint8_t)(10100100), 1, 24},
	{(uint8_t)(10100100), 1, 25},
	{(uint8_t)(10100100), 1, 26},
	{(uint8_t)(10100100), 1, 27},
	{(uint8_t)(10100100), 1, 8}
};

*/