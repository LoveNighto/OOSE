/*
 * A6.cpp
 *
 * Created: 22/12/2021 02:04:46
 * Author : Mee
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "Basics.h"
#include "DigiPort.h"
#include "LCD.h"
#include "Timer.h"
#include "QuadEncoder.h"
#include "ADConverter.h"




#define SET_H		((uint8_t)~(0b00000001))
#define SET_M		((uint8_t)~(0b00000010))
#define SET_S		((uint8_t)~(0b00000100))
#define SET_AH		((uint8_t)~(0b00001000))
#define SET_AM		((uint8_t)~(0b00010000))
#define SET_TEMP	((uint8_t)~(0b00100000))
#define SET_UNIT	((uint8_t)~(0b01000000))
#define CLR_ALARM	((uint8_t)~(0b10000000))
#define MAX_H	    ((uint8_t)(23))
#define MAX_MIN		((uint8_t)(59))
#define MAX_SEC		(MAX_MIN)


enum time_t{uhour, umin, usec, none};
enum sensor_t{utemp, uhumid, nothing};
enum temperatur_t{celcius, fahrenheit};
static void ticker_cb();
static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start);
inline uint8_t C_to_F(uint8_t val);
inline uint8_t F_to_C(uint8_t val);
bool do_update = false;
volatile uint8_t key_in = 0;
static const char headline[] PROGMEM = {"Uhrzeit    Weckzeit  Temperatur  Feuchte"};
static const char dataline[] PROGMEM = {"00:00:00    00:00        C (   )     %"};
static const char alarm_text[] PROGMEM =       {"BEEP BOOP!"};
static const char text_clear[] PROGMEM =       {"          "};
static const char temp_text[] PROGMEM =        {"HIGH TEMP!"};
	
static const char sethours[] PROGMEM =     {"Setting hours..."};
static const char setminutes[] PROGMEM =   {"Setting minutes..."};
static const char setseconds[] PROGMEM =   {"Setting seconds..."};
static const char settempalarm[] PROGMEM = {"Setting Temperature.."};
static const char setclear[] PROGMEM =     {"                     "};
/*
// Reservierung Platz fur Zahlen im EEPROM: alarm_h(0), alarm_m(1), alarm_s(2), alarm_value "Temp" (3)
uint8_t EEMEM e_sum[4];

void eeprom_write(uint8_t ew_var, uint8_t ew_pos){
	while (eeprom_is_ready()==0){}
	eeprom_write_byte(&e_sum[ew_pos],ew_var); // schreibe e_var an die Stelle e_sum[e_pos]
}

static uint8_t eeprom_read(uint8_t er_pos){
	uint8_t x;
	while (eeprom_is_ready()==0){}
	x = eeprom_read_byte(&e_sum[er_pos]);// lies ein Byte von der Stelle e_sum[er_pos]
	
	return x;
}
*/
class myClock{
	private:
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
		
		uint8_t alarm_hour;
		uint8_t alarm_min;
		uint8_t alarm_sec;
		bool alarm_status;
		
		time_t update;
		LCD* display_handle;
		
		inline void alarm_on(){
			alarm_status = true;
		}
	public:
		myClock(LCD* DisplayPort, uint8_t init_h = 0, uint8_t init_m = 0, uint8_t init_s = 0, bool alarm_status = false){
			display_handle = DisplayPort;
			// result = condition ? value1 (if yes) : value2 (else);
			hour = (init_h <= MAX_H) ? init_h : 0;
			min = (init_m <= MAX_MIN) ? init_m : 0;
			sec = (init_s <= MAX_SEC) ? init_s : 0;
			this->alarm_status = alarm_status;
			/*
			alarm_hour = eeprom_read(0);
			alarm_min = eeprom_read(1);
			alarm_sec = eeprom_read(2);
			*/
			
		}
		inline void show();
		inline void tick();
		inline void wecker();
		inline void clear_alarm();
		
		inline void set_h(uint8_t h){hour = h;}
		inline void set_m(uint8_t m){min = m;}
		inline void set_s(uint8_t s){sec = s;}
		uint8_t inline get_h(){return hour;}
		uint8_t inline get_m(){return min;}
		uint8_t inline get_s(){return sec;}
			
		inline void set_ah(uint8_t ah){
			alarm_hour = ah; 
			alarm_on();
			//eeprom_write(alarm_hour, 0);
		}
		inline void set_am(uint8_t am){
			alarm_min = am; 
			alarm_on();
			//eeprom_write(alarm_min, 1);
		}
		inline void set_as(uint8_t as){
			alarm_sec = as; 
			alarm_on();
			//eeprom_write(alarm_sec, 2);
		}
		uint8_t inline get_ah(){return alarm_hour;}
		uint8_t inline get_am(){return alarm_min;}
		uint8_t inline get_as(){return alarm_sec;}
		
		
		
};

inline void myClock::clear_alarm(){
	if(true == alarm_status){
		alarm_status = false;
		alarm_hour = 0;
		alarm_min = 0;
		alarm_sec = 0;
		display_handle->set_pos(3, 0);
		display_handle->write_FLASH_text(text_clear);
	}
}

inline void myClock::wecker(){
	if (true == alarm_status){
		if (alarm_hour == hour){
			if(alarm_min == min){
				display_handle->set_pos(0, 3);
				display_handle->write_FLASH_text(alarm_text);
			}
		}
	}
}

inline void myClock::tick(){
	sec++;
	// Tells the display to update only sec Ziffern	
	update = usec;
	if (sec > MAX_SEC){
		min++;
		sec = 0;		
		update = umin;		
	if (min > MAX_MIN){
		hour++;
		min = 0;
		update = uhour;
	if(hour > MAX_H){
		hour = 0;
		update = uhour;
			}
		}
	}
}

inline void myClock::show(){
	switch(update){
		case none:	break;
		// The order below = If uhour, updates all h,m,s. If sec, only update sec. 
		case uhour: display_handle->set_pos(1, 0);
					display_handle->write_number(hour, 2, '0');
		case umin:  display_handle->set_pos(1, 3);
					display_handle->write_number(min, 2, '0');
		case usec:	display_handle->set_pos(1, 6);
					display_handle->write_number(sec, 2, '0');
	}
	update = none;
};

class mySensor{
	private:
		LCD* display_handle; 
		uint8_t temp;
		uint8_t humid;
		uint8_t alarm_value;
		temperatur_t temp_type;
		sensor_t update;
		ADConverter* converter_handle;		//ca. 2,98V bei 25°C
		bool alarm_status;
	public:
		mySensor(LCD* DisplayPort, ADConverter* ConverterPort, temperatur_t tt = celcius){
			display_handle = DisplayPort;
			converter_handle = ConverterPort;
			temp_type = tt;
			//alarm_value = eeprom_read(3);
		}
		inline void show();
		inline void toggle_unit();
		inline void temp_alarm();
		inline void clear_alarm();
		inline void set_alarm_value(uint8_t value){
			alarm_value = value;
			alarm_status = true;
			//eeprom_write(alarm_value, 3);
		}
		uint8_t inline get_temp(){return temp;}
		
		///////////////////////////////////////////////////////// VORSICHT!!!!!!!!!!!!!!!!!!!!!!!!!!!! /////////////////////////////////////////////////////////////////
		inline void value_update(){
			if(converter_handle->value_available()){
				temp = converter_handle->temp_convert(converter_handle->get_value());
				humid = converter_handle->hum_convert(converter_handle->get_value()); 
			}
		}
		
};

inline void mySensor::show(){
	switch(update){
		case utemp:		display_handle->set_pos(1,20);
						display_handle->write_number(temp, 3);
						update = nothing;
						break;
						
		case uhumid:	display_handle->set_pos(1,34);
						display_handle->write_number(humid, 2);
						update = nothing;
						break;
						
		case nothing:	break;
						
	}
}

inline void mySensor::toggle_unit(){
	if (celcius == temp_type){
		
		temp = C_to_F(temp);
		alarm_value = C_to_F(alarm_value);
		
		display_handle->set_pos(1, 24);
		display_handle->write_char('F');
		temp_type = fahrenheit;
	}else{
		
		temp = F_to_C(temp);
		alarm_value = F_to_C(alarm_value);
		
		display_handle->set_pos(1, 24);
		display_handle->write_char('C');
		temp_type = celcius;
	}
	update = utemp;
}

inline void mySensor::clear_alarm(){
	if(alarm_status){
		alarm_status = false;
		display_handle->set_pos(3, 19);
		display_handle->write_FLASH_text(text_clear);
	}
} 

inline void mySensor::temp_alarm(){
	if (alarm_status){
		if(temp > alarm_value){
			display_handle->set_pos(3, 19);
			display_handle->write_FLASH_text(temp_text);
		}
	}
	if(temp < alarm_value){
		alarm_status = true;
	}
}

inline uint8_t C_to_F(uint8_t val){
	return (val * 9/5) + 32;
}

inline uint8_t F_to_C(uint8_t val){
	return (val - 32) * 5/9;
}

//================================================================================================
// define all Objects 
static DigiPortRaw keys(PK,SET_IN_PORT);
static DigiPortRaw leds(PA,SET_OUT_PORT);
static QuadEncoder quenc(PJ);
static LCD display(PC, LCD_Type_40x4, WRAPPING_OFF|BLINK_OFF| CURSOR_OFF | DISPLAY_ON);
static ADConverter theADConverter(uint8_t AD_CHAN_8);
static Timer16 ticker(TC1, ticker_cb);
static myClock theClock(&display);
static mySensor theSensors(&display, &theADConverter);

//================================================================================================



int main(void)
{
	sei();
	ticker.start_ms(1000);
	display.write_FLASH_text(headline);
	display.set_pos(1,0);
	display.write_FLASH_text(dataline);
	while (1){
		if (do_update) {
			theClock.show();
			theClock.wecker();
			theSensors.value_update();
			theSensors.show();
			theSensors.temp_alarm();
			do_update = false;
		}
		key_in = keys.read_raw();
		leds.write(key_in);
		/*
		switch ((uint8_t)~(key_in)){
			
			case SET_H:		theClock.set_h(read_quenc(sethours, 1, 0, 2, 0, MAX_H, theClock.get_h()));
							display.set_pos(2, 0);
							display.write_SRAM_text("HELLO");
							break;
			
			case 0b11111101:		
							display.set_pos(2, 0);
							display.write_SRAM_text("HELLO");
							theClock.set_m(read_quenc(setminutes, 1, 3, 2, 0, MAX_MIN, theClock.get_m()));
							break;
			
			
			
			case SET_S:		theClock.set_s(read_quenc(setseconds, 1, 6, 2, 0, MAX_SEC, theClock.get_s()));
							break;
			case SET_AH:	theClock.set_ah(read_quenc(sethours, 1, 12, 2, 0, MAX_H, theClock.get_ah()));
							break;
			case SET_AM:	theClock.set_am(read_quenc(setminutes, 1, 15, 2, 0, MAX_MIN, theClock.get_am()));
							theClock.set_as(0);
							break;
			case SET_TEMP:	theSensors.set_alarm_value(read_quenc(settempalarm, 1, 27, 3, 0, 50, theSensors.get_temp()));
							break;
			case SET_UNIT:	theSensors.toggle_unit();
							break;
			case CLR_ALARM: theClock.clear_alarm();
							theSensors.clear_alarm();
							break;
							
			
			
		}*/
	}
}
//================================================================================================
// ISR VORSICHT!
static void ticker_cb(void){
	theClock.tick();
	do_update = true;
}
//================================================================================================

// modify number at (line, pos) using len digits
// values range is min to max and initial value is start

static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start) {
	uint8_t val = 0;
	
	// start quadencoder and show prompt on line 3 of display
	display.set_pos(2, 0); 
	display.write_FLASH_text(prompt);
	
	// stelle Wertebereich 'min' bis 'max' ein und setzte 'start' als aktuellen Wert und starte die Funktion des Drehgebers 
	quenc.start(min, max, start); 
	
	while (! quenc.new_locked_value_available()) {
		if (quenc.new_value_available()) {
			val = (uint8_t)quenc.get_unlocked_value();
			display.set_pos(line, pos);
			display.write_number(val, len);
		}
	};
	quenc.stop();  // Stoppe den Drehgeber und loesche LEDs des Drehgebers, die evtl. noch an sind
	// when done, clear prompt and return value upon key press
	display.set_pos(2, 0);
	display.write_FLASH_text(setclear);
	return val;
	
}

