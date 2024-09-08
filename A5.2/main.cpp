/*
 * A5.1.cpp
 *
 * Created: 15/07/2022 22:17:23
 * Author : LoveNighto#1444
 
PORT A - 8 LEDs. Die LEDs sind im Kamerabild in den Tasten integriert zu sehen.
PORT B - nicht belegt
PORT C - LCD-Display der Gr??e 40 x 4; im Kamerabild zu sehen.
PORT D - 7-Segment Anzeige (100/1000-er Stellen); im Kamerabild zu sehen
PORT E - 7-Segment Anzeige (1/10-er Stellen); im Kamerabild zu sehen
PORT F - analoge Temperatur- und Feuchte-Sensoren; ?ber AD-Wandler (Kanal 5 und Kanal
3) auszulesen
PORT G - nicht belegt
PORT H - nicht belegt
PORT J - Drehgeber ? Drehknopf und Tasten sind ?ber die Web-Oberfl?che fernzusteuern; drei
Signal-LEDs sind im Kamerabild zu sehen.
PORT K - Die Tasten des Boards. Sie sind ?ber die Web-Oberfl?che fernzusteuern.

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

// EINGABETEXT
char const task_1_str[] = {"1Literally long text"};
char const task_2_str[] = {"2ABCDEF"};
char const task_3_str[] = {"3LALALA"};
char const task_4_str[] = {"4Noice! :)"};
char const test_text[11] PROGMEM = {"Test!"};
char  task_clear[] = {"                   "};

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
		uint8_t string_len;
		unsigned char erste_buchstabe;
		uint8_t string_index;
		
	public:
		Tasks(uint8_t id, BinarySemaphor* semaphoren, BoundedQueue* q, const char* str = NULL, uint8_t disp_row_pos = 0, uint8_t disp_col_pos = 0, uint8_t onoff = ON, uint8_t strlen){
			tid = id;
			status = onoff;
			semaphor = semaphoren;
			queue = q;
			string = str;
			erste_buchstabe = string[0];
			string_len = strlen;
			display_row_position = disp_row_pos;
			display_col_position = disp_col_pos;
			string_index = 1;
		}
		uint8_t getId(void){return tid;}
		uint8_t getStatus(void){return status;}
		uint8_t getDisp_Row_Pos(void){return display_row_position;}
		uint8_t getDisp_Col_Pos(void){return display_col_position;}
		BinarySemaphor* getSemaphor(void){return semaphor;}
		BoundedQueue* getQueue(void){return queue;}
		const char* getString(void){return string;}
		uint8_t getString_Index(void){return string_index;}
		unsigned char getErste_Buchstabe(void){return erste_buchstabe;}	
		uint8_t getString_len(void){return string_len;}
		
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
void inline write_to_queue(Tasks&);
void inline queue_to_display(Tasks&, Tasks&);
//================================================================================================
// OBJEKT INSTANZIEREN
DigiPortIRPT keys(PK, SET_IN_PORT);
DigiPortRaw leds(PA, SET_OUT_PORT);
LCD display(PC, LCD_Type_40x4);
Timer16 timer(TC1, blinker);
BinarySemaphor shared_resource_semaphor_1;
BinarySemaphor shared_resource_semaphor_2;
BinarySemaphor shared_resource_semaphor_3;
BoundedQueue Queue_1;
BoundedQueue Queue_2; // BoundedQueue::BoundedQueue	(uint8_t m = NO_OVERWRITE)

Tasks myTask1(task_insert(Task_1), &shared_resource_semaphor_1, &Queue_1, task_1_str, 0, 0, OFF, sizeof(task_1_str) / sizeof(task_1_str[0]));
Tasks myTask2(task_insert(Task_2), &shared_resource_semaphor_1, &Queue_1, task_2_str, 1, 0, OFF, sizeof(task_2_str) / sizeof(task_2_str[0]));
Tasks myTask3(task_insert(Task_3), &shared_resource_semaphor_2, &Queue_2, task_3_str, 2, 0, OFF, sizeof(task_3_str) / sizeof(task_3_str[0]));
Tasks myTask4(task_insert(Task_4), &shared_resource_semaphor_2, &Queue_2, task_4_str, 3, 0, OFF, sizeof(task_4_str) / sizeof(task_4_str[0]));

Tasks myViewer1(task_insert(Viewer_1), &shared_resource_semaphor_3, &Queue_1);
Tasks myViewer2(task_insert(Viewer_2), &shared_resource_semaphor_3, &Queue_2);


//================================================================================================
// Tasks und Prozeduren
void blinker(void){			// Separate Blink and IRQ //WORKING
	if(led_1.status){
		led_1.current_delay--;
		if (1 > led_1.current_delay){
			blinker_1();
			led_1.current_delay = led_1.request_delay; // repeat blink
		}
	}
	if(led_2.status){
		led_2.current_delay--;
		if (1 > led_2.current_delay){
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
		switch((uint8_t)~(keys.falling_edge())){
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
		display.set_pos(0, 0);
		status = OFF;
	}
}

void Task_1(){
	while(1){
		write_to_queue(myTask1);
		yield();
	}
}

void Task_2(){
	while(1){
		write_to_queue(myTask2);
		yield();
	}
}

void Task_3(){
	while(1){
		write_to_queue(myTask3);
		yield();
	}
}

void Task_4(){
	while(1){
		write_to_queue(myTask4);
		yield();
	}
}

void Viewer_1(){
	while(1){
		if(myTask1.getStatus()){
			queue_to_display(myViewer1, myTask1);
		}
		if(myTask2.getStatus()){
			queue_to_display(myViewer1, myTask2);
		}
		yield();
	}
}

void Viewer_2(){
	while(1){
		if(myTask3.getStatus()){
			queue_to_display(myViewer2, myTask3);
		}
		if(myTask4.getStatus()){
			queue_to_display(myViewer2, myTask4);
		}
		yield();
	}
}

void inline write_to_queue(Tasks &myTask){
	myTask.getSemaphor()->wait_aquire();
	
	if(myTask.getQueue()->get_used_size() > 0){					// MUSS ueberprufen ob que MOMENTAN LEER ist; ansonsten es wird einfach alle Buchstaben untereinander liegt und katastrophe passiert.
		myTask.getSemaphor()->release();						// Hier ist der KEY!: Que nicht leer? Nicht zu verwenden! Debuggenzeit: fast 5 std. viel spass damit? ja :>
		yield();
	} else{			
	
		myTask.getQueue()->write(myTask.getErste_Buchstabe());
		while ('\0' != myTask.getString()[myTask.getString_Index()]){
			if(myTask.getQueue()->write(myTask.getString()[myTask.getString_Index()]) == false){
				yield();
			} else{
				myTask.setString_Index(myTask.getString_Index() + 1);
			}
		}
		myTask.setString_Index(1);
		myTask.getSemaphor()->release();
	}
	
}

void inline queue_to_display(Tasks &myViewer, Tasks &myTask){
	myViewer.getSemaphor()->wait_aquire();
	unsigned char output; 
	display.set_pos(3, 0);
	display.write_number(myTask.getString_len());
	//if(output != NAC){
		//if(output == myTask.getErste_Buchstabe()){						// Weil eine Que von 2 Tasks verwendbar ist UND wir wissen NICHTS was da drin ist. Muss ich ein 'Tracker' schreiben.
			//while(myViewer.getQueue()->get_used_size()){					// MAX 10 Buchstaben/Ziffern
				output = myViewer.getQueue()->read();
				if(output == NAC){
					yield();
				} else{
					display.set_pos(myTask.getDisp_Row_Pos(), myTask.getDisp_Col_Pos());
					display.write_char(output);
					myTask.setDisp_Col_Pos(myTask.getDisp_Col_Pos() + 1);
				}
				
			//}
		//}	
		if(myTask.getString_len == myTask.getDisp_Col_Pos() + 1){
			myTask.setDisp_Col_Pos(0);
		}
		myViewer.getSemaphor()->release();
	//}
	//else{
	//	myViewer.getSemaphor()->release();
	//	yield();
	//}
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
	
	kernel();

}

