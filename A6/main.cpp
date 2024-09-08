/*
 * A6.cpp
 *
 * Created: 22/12/2021 02:04:46
 * Author : Mee
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "Basics.h"
#include "DigiPort.h"
#include "LCD.h"
#include "Timer.h"
#include "QuadEncoder.h"
#include "ADConverter.h"


#define SET_H		((uint8_t)(0b00000001))
#define SET_M		((uint8_t)(0b00000010))
#define SET_S		((uint8_t)(0b00000100))
#define SET_AH		((uint8_t)(0b00001000))
#define SET_AM		((uint8_t)(0b00010000))
#define SET_TEMP	((uint8_t)(0b00100000))
#define SET_UNIT	((uint8_t)(0b01000000))
#define CLR_ALARM	((uint8_t)(0b10000000))
#define MAX_H	    ((uint8_t)(23))
#define MAX_MIN		((uint8_t)(59))
#define MAX_SEC		(MAX_MIN)


enum time_t{uhour, umin, usec, none};
enum sensor_t{utemp, uhumid};
static void ticker_cb();
static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start);
bool do_update = false;
static const char headline[] PROGMEM = {"Uhrzeit    Weckzeit  Temperatur  Feuchte"};
static const char dataline[] PROGMEM = {"hh:mm:ss    hh:mm    xxx°C (zzz)   yy%"};
	
static const char sethours[] PROGMEM = {"Setting hours..."};
static const char setminutes[] PROGMEM = {"Setting minutes..."};
static const char setseconds[] PROGMEM = {"Setting seconds..."};
static const char settempalarm[] PROGMEM = {"Setting Temperature.."};
	



class myClock{
	private:
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
		time_t update;
		LCD* display_handle;
	protected:
		inline void reset(){hour = 0; min = 0; sec = 0;}
	public:
		myClock(LCD* DisplayPort, uint8_t init_h = 0, uint8_t init_m = 0, uint8_t init_s = 0){
			display_handle = DisplayPort;
			// result = condition ? value1 (if yes) : value2 (else);
			hour = (init_h <= MAX_H) ? init_h : 0;
			min = (init_m <= MAX_MIN) ? init_m : 0;
			sec = (init_s <= MAX_SEC) ? init_s : 0;
		}
		inline void show();
		inline void tick();
		inline void set_h(uint8_t h){hour = h;}
		inline void set_m(uint8_t m){min = m;}
		inline void set_s(uint8_t s){sec = s;}

		uint8_t inline get_h(){return hour;}
		uint8_t inline get_m(){return min;}
		uint8_t inline get_s(){return sec;}
		
		
};

class myAlarm : public myClock{
	private:
		bool isActive;
	public:
		myAlarm(LCD* Display_Port){
			myClock(	 Display_Port, 0, 0, 0);
			isActive = false;
		}

		inline void set_ah(uint8_t ah){myClock::set_h(ah);}
		inline void set_am(uint8_t am){myClock::set_m(am);}
		inline void set_as(uint8_t as){myClock::set_s(as);}
		uint8_t inline get_ah(){return myClock::get_h();}
		uint8_t inline get_am(){return myClock::get_m();}
		inline void clear_alarm(){myClock::reset();}
	};

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
					display_handle->write_number(hour);
		case umin:  display_handle->set_pos(1, 3);
					display_handle->write_number(min);
		case usec:	display_handle->set_pos(1, 6);
					display_handle->write_number(sec);
	}
	update = none;
};

class mySensor{
	private:
		LCD* display_handle; 
		uint8_t temp;
		uint8_t humid;
		uint8_t alarm_value;
	
	public:
		mySensor(LCD* DisplayPort){
			display_handle = DisplayPort;
		}
		inline void show();
		inline void set_alarm_value(uint8_t value){alarm_value = value;}
		inline void toggle_unit();
		inline void clear_alarm(){alarm_value = 0;}
		uint8_t inline get_temp(){return temp;}
		/*
		inline void value_update(){
			if(theADConverter.value_available() == true){
				temp = theADConverter.temp_convert(theADConverter.get_value());
				humid = theADConverter.hum_convert(theADConverter.get_value());
			}
		}
		*/
};
/*
void do_update(void){
	//theSensors.value_update();
}
*/

//================================================================================================
// define all Objects 
static DigiPortRaw keys(PK,SET_IN_PORT);
static QuadEncoder quenc(PJ);
static LCD display(PC, LCD_Type_40x4, WRAPPING_OFF|BLINK_OFF| CURSOR_OFF | DISPLAY_ON);
static Timer16 ticker(TC1, ticker_cb);
static myClock theClock(&display);
static mySensor theSensors(&display);
static myAlarm theAlarm(&display);
//static ADConverter theADConverter(uint8_t chan, uint8_t avg=1);
//================================================================================================

inline void mySensor::show(){}
inline void mySensor::toggle_unit(){}




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
			theSensors.show();
			do_update = false;
		}
		
		switch (keys.read_raw()) {
			
			case SET_H:		theClock.set_h(read_quenc(sethours, 1, 0, 2, 0, MAX_H, theClock.get_h()));
							break;
			case SET_M:		theClock.set_m(read_quenc(setminutes, 1, 3, 2, 0, MAX_MIN, theClock.get_m()));
							break;
			case SET_S:		theClock.set_s(read_quenc(setseconds, 1, 6, 2, 0, MAX_SEC, theClock.get_s()));
							break;
			case SET_AH:	theAlarm.set_ah(read_quenc(sethours, 1, 12, 2, 0, 23, theAlarm.get_ah()));
							break;
			case SET_AM:	theAlarm.set_am(read_quenc(setminutes, 1, 15, 2, 0, 59, theAlarm.get_am()));
							theAlarm.set_as(0);
							break;
			case SET_TEMP:	theSensors.set_alarm_value(read_quenc(settempalarm, 1, 27, 3, 0, 50, theSensors.get_temp()));
							break;
			case SET_UNIT:	theSensors.toggle_unit();
							break;
			case CLR_ALARM: theAlarm.clear_alarm();
							theSensors.clear_alarm();
							break;
			
		}
	}
}
//================================================================================================
static void ticker_cb(void){
	theClock.tick();
	do_update = true;
}
//================================================================================================

// modify number at (line, pos) using len digits
// values range is min to max and initial value is start
// when done, clear prompt and return value upon key press
static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start) {
	uint8_t val = 0;
	
	// start quadencoder and show prompt on line 3 of display
	// ROW (Horizontal) x COL (Vertical)
	display.set_pos(0, 2); 
	display.write_FLASH_text(prompt);
	
	// stelle Wertebereich 'min' bis 'max' ein und setzte 'start' als aktuellen
	// Wert und starte die Funktion des Drehgebers
	quenc.start(min, max, start); 
	
	while (! quenc.new_locked_value_available()) {
		if (quenc.new_value_available()) {
			val = (uint8_t)quenc.get_unlocked_value();
			display.set_pos(pos, line);
			display.write_number(val, len);
		}
	};
	quenc.stop();  // Stoppe den Drehgeber und loesche LEDs des Drehgebers, die evtl. noch an sind
	return val;
	
}

