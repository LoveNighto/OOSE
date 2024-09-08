/*
 * A5.cpp
 *
 * Created: 19/12/2021 14:35:15
 * Author : Mee
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "Basics.h"
#include "DigiPort.h"
#include "LCD.h"
#include "Timer.h"
#include "OSKernel.h"
#include "BinarySemaphor.h"
#include "BoundedQueue.h"

#define F_CPU 16000000LU
#define ON ((uint8_t)(1))
#define OFF ((uint8_t)(0))

void Blinker();	// Timer gesteuerte Prozeduren
void Key_Cntl();	// 7 Task-Kernel gesteuert
void Task_1();
void Task_2();
void Task_3();
void Task_4();
void Viewer_1();
void Viewer_2();
void Activ_Deactiv(uint8_t id);	// normale prozedur

DigiPortRaw keys(PA, SET_IN_PORT);
DigiPortRaw leds(PB, SET_OUT_PORT);
LCD display(PC, LCD_Type_24x2);
Timer16 timer(TC1, Blinker);
BinarySemaphor shared_resource_semaphor_1;
BinarySemaphor shared_resource_semaphor_2;
BoundedQueue Queue_1;
BoundedQueue Queue_2;

volatile uint8_t key_in = 0;

typedef struct{uint8_t tid;} task_type;

typedef struct{
	uint8_t status;
	uint8_t current_delay;
	uint8_t request_delay;
} led_type;

task_type task[8] = {0, 0, 0, 0, 0, 0, 0}; 
led_type led_1 = {OFF, 0, 0};
led_type led_2 = {OFF, 0, 0};
	
char const t1_str[11] PROGMEM = {"HelloWorld"};
char const t1_nope[2] PROGMEM = {"F"};
char const t1_fope[2] PROGMEM = {"X"};
char const t1_yope[2] PROGMEM = {"Y"};
char const t1_fehler[2] PROGMEM = {"N"};
char t1_ausgabe[11] ={};

void Blinker(){
	if(led_1.status == ON){
		if (led_1.current_delay == 0){
			leds.toggle(0b00001111);
			led_1.current_delay = led_1.request_delay; // repeat blink
		}else if(led_1.current_delay > 0){
			led_1.current_delay--;
		}
	}
	if(led_2.status == ON){
		if (led_2.current_delay == 0){
			leds.toggle(~(0b00001111));
			led_2.current_delay = led_2.request_delay; // repeat blink
		}else if(led_2.current_delay > 0){
			led_2.current_delay--;
		}
	}
}

void Task_1(){
	while(1){
		shared_resource_semaphor_1.wait_aquire();
		uint8_t i = 0;
		Queue_1.clear();
		for(i=0; i< 11; i++){
			//display.write_number(i);
			if(Queue_1.write(t1_str[i]) == false){				// GOOD but Queue is Leer
				//display.write_FLASH_text(t1_nope); // F
				//i--;
				yield();
			}
		}
		shared_resource_semaphor_1.release();
		yield();
	}
}

void Viewer_1(){
	while(1){
		shared_resource_semaphor_1.wait_aquire();
		display.set_pos(0, 0);
		for(uint8_t i=0; i< 11; i++){
			if(Queue_1.read() == NAC){
				//display.write_FLASH_text(t1_fehler); //N	// GOOD no i--
				yield();
			}else{
				//display.write_FLASH_text(t1_fope);		//X // NEVER GONE HERE WHY XD
				//t1_ausgabe[i] = Queue_1.read();
				//display.write_char(t1_ausgabe[i]);
				display.write_char(Queue_1.read());
			}
		}
		shared_resource_semaphor_1.release();
		yield();
	}
}

void Activ_Deactiv(uint8_t id) {
	if (is_active(id) == 0){		// wenn task deactive -> active
		activate(id);
	}
	else{
		deactivate(id);
	}
}

void Key_Cntl(){
	while (1) {
		key_in = keys.read_raw();
		switch ((uint8_t)~(key_in)) {
			
			case 0b11111110:
			Activ_Deactiv(task[0].tid);
			break;
			
			case 0b11111101:
			//Activ_Deactiv(task[1].tid);
			break;
			
			case 0b11111011:
			//Activ_Deactiv(task[2].tid);
			break;
			
			case 0b11110111:
			//Activ_Deactiv(task[3].tid);
			break;

			case 0b11101111:
			led_1.status = ON;
			led_1.request_delay = 1;
			led_1.current_delay = led_1.request_delay;
			break;
			
			case 0b11011111:
			led_1.status = ON;
			led_1.request_delay = 10;
			led_1.current_delay = led_1.request_delay;
			break;
			
			case 0b10111111:
			led_2.status = ON;
			led_2.request_delay = 2;
			led_2.current_delay = led_2.request_delay;
			break;
			
			case 0b01111111:
			led_2.status = ON;
			led_2.request_delay = 10;
			led_2.current_delay = led_2.request_delay;
			break;
		}
		yield();
	}
}

int main(void) {
	leds.off();
	timer.start(100); // in ms
	task[0].tid = task_insert(Task_1, Medium);
	/*
	task[1].tid = task_insert(Task_2, Low);
	task[2].tid = task_insert(Task_3, Low);
	task[3].tid = task_insert(Task_4, Low);
	*/
	
	task[4].tid = task_insert(Key_Cntl, High);
	task[5].tid = task_insert(Viewer_1, Medium);
	//task[6].tid = task_insert(Viewer_2, High);
	kernel();
}