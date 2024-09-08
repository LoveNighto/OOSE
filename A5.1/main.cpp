/*
 * A5.1.cpp
 *
 * Created: 15/07/2022 22:17:23
 * Author : Mee
 
PORT A – 8 LEDs. Die LEDs sind im Kamerabild in den Tasten integriert zu sehen.
PORT B – nicht belegt
PORT C – LCD-Display der Größe 40 x 4; im Kamerabild zu sehen.
PORT D – 7-Segment Anzeige (100/1000-er Stellen); im Kamerabild zu sehen
PORT E – 7-Segment Anzeige (1/10-er Stellen); im Kamerabild zu sehen
PORT F – analoge Temperatur- und Feuchte-Sensoren; über AD-Wandler (Kanal 5 und Kanal
3) auszulesen
PORT G – nicht belegt
PORT H – nicht belegt
PORT J – Drehgeber – Drehknopf und Tasten sind über die Web-Oberfläche fernzusteuern; drei
Signal-LEDs sind im Kamerabild zu sehen.
PORT K – Die Tasten des Boards. Sie sind über die Web-Oberfläche fernzusteuern.

/////////////////////////////// ALL IN RUNTIME ////////////////////////////////////////


REMOTE DESKTOP

KLB304-REMOTExx

user: uCRemote
pass: moonshine
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
//================================================================================================
// GLOBALE VARIABLE
volatile uint8_t key_in = 0;

// EINGABETEXT
char const task_1_str[] = {"1Literallytext"};
char const task_2_str[] = {"2ABCDEF"};
char const test_text[11] PROGMEM = {"Test!"};

//================================================================================================
// NOTWENDIGE Typen/ Classen
class Tasks{
	private:
		uint8_t tid;
		uint8_t status;
		BinarySemaphor* semaphor;
		BoundedQueue* queue;
		uint8_t display_row_position;
		uint8_t display_col_position;
		const char* string;
		uint8_t string_index;
		
	public:
		Tasks(uint8_t id, BinarySemaphor* semaphoren, BoundedQueue* q, const char* str = NULL, uint8_t disp_row_pos = 0, uint8_t disp_col_pos = 0, uint8_t onoff = ON){
			tid = id;
			status = onoff;
			semaphor = semaphoren;
			queue = q;
			string = str;
			display_row_position = disp_row_pos;
			display_col_position = disp_col_pos;
			string_index = 0;
		}
		uint8_t getId(void){return tid;}
		uint8_t getStatus(void){return status;}
		uint8_t getDisp_Row_Pos(void){return display_row_position;}
		uint8_t getDisp_Col_Pos(void){return display_col_position;}
		BinarySemaphor* getSemaphor(void){return semaphor;}
		BoundedQueue* getQueue(void){return queue;}
		const char* getString(void){return string;}
		uint8_t getString_Index(void){return string_index;}
		
		void setStatus(uint8_t st){status = st;}
		void setSemaphor(BinarySemaphor* sem){semaphor = sem;}
		void setQueue(BoundedQueue* q){queue = q;}
		void setDisp_Row_Pos(uint8_t drp){display_row_position = drp;}
		void setDisp_Col_Pos(uint8_t dcp){display_col_position = dcp;}
		void setString(const char* str){string = str;}
		void setString_Index(uint8_t si){string_index = si;}
		
		void On_off(void);
		
};	
typedef struct{
	uint8_t status;
	uint8_t current_delay;
	uint8_t request_delay;
} led_type;

led_type led_1 = {OFF, 0, 0};
led_type led_2 = {OFF, 0, 0};

//================================================================================================
// PROZEDUR INIT
void blinker(void);	
void inline blinker_1(void);
void inline blinker_2(void);
void Key_Ctl(void);
void Task_1(void);
void Task_2(void);
void Task_3(void);
void Task_4(void);
void Viewer_1(void);
void Viewer_2(void);
//================================================================================================
// OBJEKT INSTANZIEREN
DigiPortRaw keys(PK, SET_IN_PORT);
DigiPortRaw leds(PA, SET_OUT_PORT);
LCD display(PC, LCD_Type_40x4);
Timer16 timer(TC1, blinker);
BinarySemaphor shared_resource_semaphor_1;
BinarySemaphor shared_resource_semaphor_2;
BoundedQueue Queue_1;
BoundedQueue Queue_2; // BoundedQueue::BoundedQueue	(uint8_t m = NO_OVERWRITE)

Tasks myTask1(task_insert(Task_1,Low), &shared_resource_semaphor_1, &Queue_1, task_1_str, 0, 0, OFF);
Tasks myTask2(task_insert(Task_2,Low), &shared_resource_semaphor_1, &Queue_1, task_2_str, 1, 0, OFF);
Tasks myTask3(task_insert(Task_3,Low), &shared_resource_semaphor_2, &Queue_2, "3LALALA", 2, 0, OFF);
Tasks myTask4(task_insert(Task_4,Low), &shared_resource_semaphor_2, &Queue_2, "4Noice! :)", 3, 0, OFF);


Tasks myViewer1(task_insert(Viewer_1,Low), &shared_resource_semaphor_1, &Queue_1);
Tasks myViewer2(task_insert(Viewer_2,Low), &shared_resource_semaphor_2, &Queue_2);


//================================================================================================
// Tasks und Prozeduren
void blinker(void){			// Separate Blink and IRQ //WORKING
	if(led_1.status == ON){
		led_1.current_delay--;
		if (0 == led_1.current_delay){
			blinker_1();
			led_1.current_delay = led_1.request_delay; // repeat blink
		}
	}
	if(led_2.status == ON){
		led_2.current_delay--;
		if (0 == led_2.current_delay){
			blinker_2();
			led_2.current_delay = led_2.request_delay; 
		}
	}
}

void inline blinker_1(void){
	leds.toggle(0b00001111);
}

void inline blinker_2(void){
	leds.toggle(~(0b00001111));
}

void Key_Ctl(void){			//WORKING
	while(1){
		key_in = keys.read_raw();
		switch((uint8_t)~(key_in)){
			case 0b11111110:
			myTask1.On_off();
			break;
			
			case 0b11111101:
			myTask2.On_off();
			break;
			
			case 0b11111011:
			myTask3.On_off();
			break;
			
			case 0b11110111:
			myTask4.On_off();
			break;
			
			case 0b11101111:							// Taste 4 LEFT SIDE 
			led_1.status = ON;
			led_1.request_delay = 1;
			led_1.current_delay = led_1.request_delay;
			break;
			
			case 0b11011111:							// Taste 5
			led_1.status = ON;
			led_1.request_delay = 3;
			led_1.current_delay = led_1.request_delay;
			break;
			
			case 0b10111111:							// Taste 6 RIGHTSIDE
			led_2.status = ON;
			led_2.request_delay = 2;
			led_2.current_delay = led_2.request_delay;
			break;
			
			case 0b01111111:							// Taste 7
			led_2.status = ON;
			led_2.request_delay = 4;
			led_2.current_delay = led_2.request_delay;
			break;
			
		}
	}
}

void Tasks::On_off(){
	if(false == is_active(tid)){
		
		activate(tid);
		status = ON;
	}else{
		deactivate(tid);
		status = OFF;
	}
}

void Task_1(){
	while(1){
		if(myTask1.getStatus() == ON){
			myTask1.getSemaphor()->wait_aquire();
			myTask1.getQueue()->clear();
			while ('\0' != myTask1.getString()[myTask1.getString_Index()]){

				if(myTask1.getQueue()->write(myTask1.getString()[myTask1.getString_Index()]) == false){
					myTask1.setString_Index(myTask1.getString_Index() - 1);
					yield();
				}
				else{
					myTask1.setString_Index(myTask1.getString_Index() + 1);
				}
			}
			myTask1.setString_Index(0);
			myTask1.getSemaphor()->release();
			//myTask1.On_off();
			yield();
		}
	}
}

void Task_2(){
	while(1){
		if(myTask2.getStatus() == ON){
			myTask2.getSemaphor()->wait_aquire();
			myTask2.getQueue()->clear();
			while ('\0' != myTask2.getString()[myTask2.getString_Index()]){

				if(myTask2.getQueue()->write(myTask2.getString()[myTask2.getString_Index()]) == false){
					myTask2.setString_Index(myTask2.getString_Index() - 1);
					yield();
				}
				else{
					myTask2.setString_Index(myTask2.getString_Index() + 1);
				}
			}
			myTask2.setString_Index(0);
			myTask2.getSemaphor()->release();
			//myTask2.On_off();
			yield();
		}
	}
}

void Task_3(){
	while(1){
		if(myTask3.getStatus() == ON){
			myTask3.getSemaphor()->wait_aquire();
			myTask3.getQueue()->clear();
			while ('\0' != myTask3.getString()[myTask3.getString_Index()]){

				if(myTask3.getQueue()->write(myTask3.getString()[myTask3.getString_Index()]) == false){
					myTask3.setString_Index(myTask3.getString_Index() - 1);
					yield();
				}
				else{
					myTask3.setString_Index(myTask3.getString_Index() + 1);
				}
			}
			myTask3.setString_Index(0);
			myTask3.getSemaphor()->release();
			//myTask3.On_off();
			yield();
		}
	}
}

void Task_4(){
	while(1){
		if(myTask4.getStatus() == ON){
			myTask4.getSemaphor()->wait_aquire();
			myTask4.getQueue()->clear();
			while ('\0' != myTask4.getString()[myTask4.getString_Index()]){

				if(myTask4.getQueue()->write(myTask4.getString()[myTask4.getString_Index()]) == false){
					myTask4.setString_Index(myTask4.getString_Index() - 1);
					yield();
				}
				else{
					myTask4.setString_Index(myTask4.getString_Index() + 1);
				}
			}
			myTask4.setString_Index(0);
			myTask4.getSemaphor()->release();
			//myTask4.On_off();
			yield();
		}
	}
}

void Viewer_1(){
	while(1){
		//myViewer1.getSemaphor()->wait_aquire();
		//display.set_pos(3,0);									// ZUM VERSUCH
		//display.write_number(Queue_1.get_used_size(), 1);
		unsigned char output = myViewer1.getQueue()->read();
		
		if (output == '1'){
			while(myViewer1.getQueue()->get_used_size()){
				output = myViewer1.getQueue()->read();
				display.set_pos(myTask1.getDisp_Row_Pos(), myTask1.getDisp_Col_Pos());
				display.write_char(output);
				myTask1.setDisp_Col_Pos(myTask1.getDisp_Col_Pos() + 1);
			}
			myTask1.setDisp_Col_Pos(0);
		}
		if(output == '2'){
			while(myViewer1.getQueue()->get_used_size()){
				output = myViewer1.getQueue()->read();
				display.set_pos(myTask2.getDisp_Row_Pos(), myTask2.getDisp_Col_Pos());
				display.write_char(output);
				myTask2.setDisp_Col_Pos(myTask2.getDisp_Col_Pos() + 1);
			}
			myTask2.setDisp_Col_Pos(0);
		}
		else{
			yield();
		}
		//myViewer1.getSemaphor()->release();
		yield();
	}
}

void Viewer_2(){
	while(1){
		//myViewer2.getSemaphor()->wait_aquire();
		//display.set_pos(3,0);									// ZUM VERSUCH
		//display.write_number(Queue_1.get_used_size(), 1);
		unsigned char output = myViewer2.getQueue()->read();
		
		if (output == '3'){
			while(myViewer2.getQueue()->get_used_size()){
				output = myViewer2.getQueue()->read();
				display.set_pos(myTask3.getDisp_Row_Pos(), myTask3.getDisp_Col_Pos());
				display.write_char(output);
				myTask3.setDisp_Col_Pos(myTask3.getDisp_Col_Pos() + 1);
			}
			myTask3.setDisp_Col_Pos(0);
		}
		
		if(output == '4'){
			while(myViewer2.getQueue()->get_used_size()){
				output = myViewer2.getQueue()->read();
				display.set_pos(myTask4.getDisp_Row_Pos(), myTask4.getDisp_Col_Pos());
				display.write_char(output);
				myTask4.setDisp_Col_Pos(myTask4.getDisp_Col_Pos() + 1);
			}
			myTask4.setDisp_Col_Pos(0);
		}
		
		else{
			yield();
		}
		//myViewer2.getSemaphor()->release();
		yield();
	}
}

//================================================================================================
int main(void)
{
	timer.start_ms(500);
	
	task_insert(Key_Ctl, High);
	display.set_pos(0, 0);
	display.clear();
	leds.off();
	deactivate(myTask1.getId());
	deactivate(myTask2.getId());
	deactivate(myTask3.getId());
	deactivate(myTask4.getId());
	
	kernel(JitterControl);

}

