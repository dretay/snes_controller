// 
// 
// 

#include "snes_controller.h"

// FUNCTIONS
void strobe(void){
	digitalWrite(DATA_LATCH, HIGH);
	delayMicroseconds(12);
	digitalWrite(DATA_LATCH, LOW);
}

int shiftin(void){
	int ret = digitalRead(DATA_SERIAL);
	delayMicroseconds(12);
	digitalWrite(DATA_CLOCK, HIGH);
	delayMicroseconds(12);
	digitalWrite(DATA_CLOCK, LOW);
	return ret;
}

int buttons(void){
	int ret = 0;
	byte i;
	strobe();
	for (i = 0; i < 16; i++) {
		ret |= shiftin() << i;
	}
	return ~ret;
}

void setup_controller(void){
	pinMode(DATA_LATCH, OUTPUT);
	pinMode(DATA_CLOCK, OUTPUT);
	pinMode(DATA_SERIAL, INPUT);
	digitalWrite(DATA_LATCH, HIGH);
	digitalWrite(DATA_CLOCK, HIGH);
}
