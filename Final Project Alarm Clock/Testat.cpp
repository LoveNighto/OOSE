/*
 * A6.cpp
 *
 * Created: 22/12/2021 02:04:46
 * Author : Erlangga, Fabian B.
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
/*
PORT A � 8 LEDs. Die LEDs sind im Kamerabild in den Tasten integriert zu sehen.
PORT B � nicht belegt
PORT C � LCD-Display der Gr��e 40 x 4; im Kamerabild zu sehen.
PORT D � 7-Segment Anzeige (100/1000-er Stellen); im Kamerabild zu sehen
PORT E � 7-Segment Anzeige (1/10-er Stellen); im Kamerabild zu sehen
PORT F � analoge Temperatur- und Feuchte-Sensoren; �ber AD-Wandler (Kanal 5 und Kanal
3) auszulesen
PORT G � nicht belegt
PORT H � nicht belegt
PORT J � Drehgeber � Drehknopf und Tasten sind �ber die Web-Oberfl�che fernzusteuern; drei
Signal-LEDs sind im Kamerabild zu sehen.
PORT K � Die Tasten des Boards. Sie sind �ber die Web-Oberfl�che fernzusteuern.
*/



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

// Einige Objekte hier wird in den Klassen benoetigt.
// Objekt Instanzieren
static DigiPortRaw leds(PA,SET_OUT_PORT);
static DigiPortRaw keys(PK,SET_IN_PORT);
static QuadEncoder quenc(PJ);
static LCD display(PC, LCD_Type_40x4, WRAPPING_OFF|BLINK_OFF| CURSOR_OFF | DISPLAY_ON);
static ADConverter humidADConverter(AD_CHAN_5);
static ADConverter tempADConverter(AD_CHAN_3);

// Enumeration
enum time_t{uhour, umin, usec, ualarm, none};
enum sensor_t{utemp, uhumid, nothing};
enum temperatur_t{celcius, fahrenheit};

// Funktionen/Prozeduren Init
static void ticker_cb();
static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start);
inline uint16_t C_to_F(uint16_t val);
inline uint16_t F_to_C(uint16_t val);
inline void led_blinken();

// Globale Var.
bool do_blink = false;
bool do_update = false;
uint8_t sensor_delay = 5;
uint8_t led_delay = 2;

// Texte
static const char headline[] PROGMEM = {"Uhrzeit    Weckzeit  Temperatur  Feuchte"};
static const char dataline[] PROGMEM = {"00:00:00    00:00        C (xxx)   yy  %"};
static const char alarm_text[] PROGMEM =	{"BEEP BOOP!"};
static const char temp_text[] PROGMEM  =    {"HIGH TEMP!"};
static const char text_clear[] PROGMEM =	{"          "};
static const char sethours[] PROGMEM =     {"Setting hours..."};
static const char setminutes[] PROGMEM =   {"Setting minutes..."};
static const char setseconds[] PROGMEM =   {"Setting seconds..."};
static const char settempalarm[] PROGMEM = {"Setting Temperature.."};
static const char setclear[] PROGMEM =     {"                     "};

// Reservierung Platz fur Zahlen im EEPROM: alarm_h(0), alarm_m(1), alarm_s(2), alarm_value "Temp" (3)
uint8_t EEMEM e_sum[4];

void eeprom_write(uint8_t ew_var, uint8_t ew_pos){
	while (eeprom_is_ready()==0){}
	eeprom_write_byte(&e_sum[ew_pos],ew_var); // schreibe e_var an die Stelle e_sum[e_pos]
}

static uint8_t eeprom_read(uint8_t er_pos){
	uint8_t x;
	while (eeprom_is_ready() == 0){}
	x = eeprom_read_byte(&e_sum[er_pos]);// lies ein Byte von der Stelle e_sum[er_pos]
	
	return x;
}

class myClock{
	private:
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
		
		uint8_t alarm_hour;
		uint8_t alarm_min;
		uint8_t alarm_sec;
		bool alarm_status;
		bool alarm_text_status;
		
		time_t update;
		LCD* display_handle;
		
		inline void alarm_on(){alarm_status = true;}
	public:
		myClock(LCD* DisplayPort, uint8_t init_h = 0, uint8_t init_m = 0, uint8_t init_s = 0, bool alarm_status = false){
			display_handle = DisplayPort;
			// result = condition ? value1 (if yes) : value2 (else);
			hour = (init_h <= MAX_H) ? init_h : 0;
			min = (init_m <= MAX_MIN) ? init_m : 0;
			sec = (init_s <= MAX_SEC) ? init_s : 0;
			this->alarm_status = alarm_status;
			alarm_text_status = false;
			alarm_hour = eeprom_read(0);
			alarm_min = eeprom_read(1);
			alarm_sec = eeprom_read(2);
			
		}
		void show();
		inline void tick();
		inline void wecker();
		inline void clear_alarm();
		
		inline void set_update(time_t refresh_time){update = refresh_time;}
		inline void set_h(uint8_t h){hour = h;}
		inline void set_m(uint8_t m){min = m;}
		inline void set_s(uint8_t s){sec = s;}
		uint8_t inline get_h(){return hour;}
		uint8_t inline get_m(){return min;}
		uint8_t inline get_s(){return sec;}
			
		inline void set_ah(uint8_t ah){
			alarm_hour = ah; 
			alarm_on();
			eeprom_write(alarm_hour, 0);
		}
		inline void set_am(uint8_t am){
			alarm_min = am; 
			alarm_on();
			eeprom_write(alarm_min, 1);
		}
		inline void set_as(uint8_t as){
			alarm_sec = as; 
			alarm_on();
			eeprom_write(alarm_sec, 2);
		}
		uint8_t inline get_ah(){return alarm_hour;}
		uint8_t inline get_am(){return alarm_min;}
		uint8_t inline get_as(){return alarm_sec;}
};

inline void myClock::clear_alarm(){
	if(alarm_status){
		alarm_status = false;
		// alarm_hour = 0; //alarm_min = 0; //alarm_sec = 0;
		// hier alarm wird nicht zurueckgesetzt sondern bleibt in EEPROM und automatisch eingeschatet.
		update = ualarm;
		do_blink = false;
		alarm_text_status = false;
		display_handle->set_pos(3, 0);
		display_handle->write_FLASH_text(text_clear);
	}
}

inline void myClock::wecker(){
	if (alarm_text_status){
		display_handle->set_pos(3, 0);
		display_handle->write_FLASH_text(alarm_text);
	}
	else if (alarm_status){
		if (alarm_hour == hour){
			if(alarm_min == min){
				// hier wird die Alarm text BLEIBT angezeigt bis die Funktion clear_alarm() aufgerufen wird.  (der Alarm-Text nicht nur in dieselbe min angezeigt)
				alarm_text_status = true;
				do_blink = true;
			}
		}
	}
	// nach eine Minute vor der Ausschaltung des Alarms wird die Alarm-Funktion AUTOMATISCH eingeschaltet.
	else if (alarm_min > min){
		alarm_status = true;
	}
}

inline void myClock::tick(){
	sec++;
	// Hier enum update hilft uns nur per Ziffern auf dem Display zu aktualisieren.
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

void myClock::show(){
	switch(update){
		case none:	break;
		// The order below = If uhour, updates all h,m,s. If sec, only update sec. 
		// Die Reihenfolge unten = Wenn uhour, aktualisiert alle h,m,s. Wenn sec, nur sec wird aktualisiert.
		case uhour:		display_handle->set_pos(1, 0);
						display_handle->write_number(hour, 2, '0');
		case umin:		display_handle->set_pos(1, 3);
						display_handle->write_number(min, 2, '0');
		case usec:		display_handle->set_pos(1, 6);
						display_handle->write_number(sec, 2, '0');
		case ualarm:	display_handle->set_pos(1, 12);
						display_handle->write_number(alarm_hour, 2, '0');
						display_handle->set_pos(1, 15);
						display_handle->write_number(alarm_min, 2, '0');
		
	}
	update = none;
};

class mySensor{
	private:
		LCD* display_handle; 
		uint16_t temp_inC;				//BASE wird gespeichert, wird fuer Alarmvergleichung benoetigt;
		uint16_t temp_inF;				// nur fuer Anzeige
		uint16_t humid;
		uint16_t alarm_value_inC;		//BASE wird gespeichert.
		uint16_t alarm_value_inF;		// nur fuer Anzeige
		temperatur_t temp_type;
		sensor_t update;
		ADConverter* converter_handle_temp;		//ca. 2,98V bei 25�C
		ADConverter* converter_handle_hum;
		bool alarm_status;
	public:
		mySensor(LCD* DisplayPort, ADConverter* ConverterTemp, ADConverter* ConverterHum, temperatur_t tt = celcius){
			display_handle = DisplayPort;
			converter_handle_temp = ConverterTemp;
			converter_handle_hum = ConverterHum;
			temp_type = tt;
			alarm_value_inC = eeprom_read(3);
			alarm_value_inF = C_to_F(alarm_value_inC);
		}
		
		void show();
		inline void toggle_unit();
		inline void temp_alarm();
		inline void clear_alarm();
		inline void set_alarm_value(uint16_t);
		
		uint8_t inline get_temp_inC(){return temp_inC;}
		uint8_t inline get_temp_inF(){return temp_inF;}
		
		uint8_t inline get_alarm_value(){
			// result = condition ? value1 (if yes) : value2 (else);
			return (celcius == temp_type) ? alarm_value_inC : alarm_value_inF;
		}
		
		void value_update(){
			uint16_t input_temp = 0;
			// Sensoren werden nur alle 5 sec aktualisert.
			if(1 > sensor_delay){
				if(converter_handle_hum->value_available()){
					humid = converter_handle_hum->hum_convert(converter_handle_hum->get_value());
					update = uhumid;
				}
				if(converter_handle_temp->value_available()){
					input_temp = converter_handle_temp->temp_convert(converter_handle_temp->get_value());
					temp_inF = C_to_F(input_temp);
					temp_inC = input_temp;

					leds.toggle(0b00000001);
					update = utemp;
				}
				sensor_delay = 5;
			}
			else if(5 < sensor_delay){
				sensor_delay = 5;
			}
		}
};

inline void mySensor::set_alarm_value(uint16_t value){
	if(celcius == temp_type){
		alarm_value_inC = value;
	}
	else{
		alarm_value_inF = value;
		alarm_value_inC = F_to_C(value);
	}
	alarm_status = true;
	eeprom_write(alarm_value_inC, 3);
}

void mySensor::show(){
	switch(update){
		// Hier utemp und uhumid kann getauscht werden. Es hängt davon ab, welche Sensoren mehr Aktualisierung braucht.
		// Momentan habe ich so geschrieben, wenn temp aktualisert, so ist auch humid.
		case utemp:		display_handle->set_pos(1,22);
						if (celcius == temp_type){
							display_handle->write_number(temp_inC, 2, '0');
						}else{
							display_handle->write_number(temp_inF, 2, '0');
						}
						
						display_handle->set_pos(1,28);
						if(celcius == temp_type){
							display_handle->write_number(alarm_value_inC, 3, '0');
						}else{
							display_handle->write_number(alarm_value_inF, 3, '0');
						}
						//break;
						
		case uhumid:	display_handle->set_pos(1,35);
						display_handle->write_number(humid, 2);
						break;
						
		case nothing:	break;
						
	}
	update = nothing;
}

inline void mySensor::toggle_unit(){
	if (celcius == temp_type){

		display_handle->set_pos(1, 25);
		display_handle->write_char('F');
		temp_type = fahrenheit;
	}else{
		
		display_handle->set_pos(1, 25);
		display_handle->write_char('C');
		temp_type = celcius;
	}
	update = utemp;
}

inline void mySensor::clear_alarm(){
	if(alarm_status){
		alarm_status = false;
		do_blink = false;
		display_handle->set_pos(3, 19);
		display_handle->write_FLASH_text(text_clear);
		
	}
} 

inline void mySensor::temp_alarm(){
	if (alarm_status){
		if(temp_inC > alarm_value_inC){
			do_blink = true;
			display_handle->set_pos(3, 19);
			display_handle->write_FLASH_text(temp_text);
			
		}
	}
	// Temperature-Alarm wird unter untengeschriebene Bedingung AUTOMATISCH eingeschaltet.
	else if(temp_inC < alarm_value_inC){
		alarm_status = true;
	}
}

inline uint16_t C_to_F(uint16_t val){
	return (val * 9/5) + 32;
}

inline uint16_t F_to_C(uint16_t val){
	return (val - 32) * 5/9;
}

inline void led_blinken(){				// --BUG: Nachdem Benutzung des Drehgebers, ist die Funktion nicht aufgerufen.-- (geloescht)
	if (1 > led_delay){
		if(do_blink){
			leds.toggle(0b00111000);
			display.set_pos(3, 0);
			display.write_FLASH_text(text_clear);
			display.set_pos(3, 19);
			display.write_FLASH_text(text_clear);
		}
		led_delay = 2;
	}
	// Manchmal wenn man die Drehgeberfunktion so lange benutzt, hat sich der delay zu minus führt weil 
	// delay-- jede 1s in ISR.
	// und hier benutzen wir uint8_t d.h -1 = 255.
	else if(led_delay > 2){
		led_delay = 2;
	}
}


// define all Objects
static Timer16 ticker(TC1, ticker_cb);
static myClock theClock(&display, 0, 0, 0, true);
static mySensor theSensors(&display, &tempADConverter, &humidADConverter);



int main(void)
{
	sei();
	ticker.start_ms(1000);
	display.write_FLASH_text(headline);
	display.set_pos(1,0);
	display.write_FLASH_text(dataline);
	while (1){
		if (do_update) {
			leds.toggle(0b10000000);
			theClock.wecker();
			theClock.show();
			theSensors.value_update();
			theSensors.temp_alarm();
			theSensors.show();
			led_blinken();
			leds.toggle(0b10000000);
			do_update = false;
		}
		
																/*
																Uhrzeit    Weckzeit  Temperatur  Feuchte
																00:00:00    00:00    xxx�C (zzz)   xx%
		
																<Weckeralarm>       <Temperaturalaram>
																*/
		leds.write(keys.read_raw());
		switch (keys.read_raw()) {
			//										(text, vertical, horizontal, maxlen,  min, max, start)
			case SET_H:		theClock.set_h(read_quenc(sethours, 1, 0, 2, 0, MAX_H, theClock.get_h())); //0
							break;
			case SET_M:		theClock.set_m(read_quenc(setminutes, 1, 3, 2, 0, MAX_MIN, theClock.get_m()));  //1
							break;
			case SET_S:		theClock.set_s(read_quenc(setseconds, 1, 6, 2, 0, MAX_SEC, theClock.get_s()));  //2
							break; 
			case SET_AH:	theClock.set_ah(read_quenc(sethours, 1, 12, 2, 0, MAX_H, theClock.get_ah()));  //3
							break;
			case SET_AM:	theClock.set_am(read_quenc(setminutes, 1, 15, 2, 0, MAX_MIN, theClock.get_am()));  //4
							theClock.set_as(0);
							break;
			case SET_TEMP:	theSensors.set_alarm_value(read_quenc(settempalarm, 1, 28, 3, 0, 122, theSensors.get_alarm_value()));  //5
							break;
			case SET_UNIT:	theSensors.toggle_unit();  //6
							break;
			case CLR_ALARM: theClock.clear_alarm();  //7
							theSensors.clear_alarm();
							break;
			
		}
	}
}
//================================================================================================
// ISR 
static void ticker_cb(void){
	theClock.tick();
	sensor_delay--;
	led_delay--;
	do_update = true;
}
//================================================================================================

// modify number at (line, pos) using len digits
// values range is min to max and initial value is start

static uint8_t read_quenc (const char* prompt, uint8_t line, uint8_t pos, uint8_t len, uint8_t min, uint8_t max, int8_t start) {
	uint8_t val = start;
	
	// start quadencoder and show prompt on line 3 of display
	// ROW (Horizontal) x COL (Vertical)
	display.set_pos(2, 0); 
	display.write_FLASH_text(prompt);
	
	// stelle Wertebereich 'min' bis 'max' ein und setzte 'start' als aktuellen Wert und starte die Funktion des Drehgebers 
	quenc.start(min, max, start); 
	
	while (! quenc.new_locked_value_available()) {
		if (quenc.new_value_available()) {
			val = (uint8_t)quenc.get_unlocked_value();
			display.set_pos(line, pos);
			display.write_number(val, len, '0');
		}
	};
	quenc.stop();  // Stoppe den Drehgeber und loesche LEDs des Drehgebers, die evtl. noch an sind
	// when done, clear prompt and return value upon key press
	display.set_pos(2, 0);
	display.write_FLASH_text(setclear);
	return val;
	
}

