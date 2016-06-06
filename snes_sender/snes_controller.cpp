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

bool is_valid(int state){
	bool valid = false;
	if (state & SNES_B) { 
		printf("B ");
		valid = true;
	}
	if (state & SNES_Y) { 
		printf("Y ");
		valid = true;
	}
	if (state & SNES_SELECT) { 
		printf("SELECT ");
		valid = true;
	}
	if (state & SNES_START) { 
		printf("START ");
		valid = true;
	}
	if (state & SNES_UP) { 
		printf("UP ");
		valid = true;
	}
	if (state & SNES_DOWN) { 
		printf("DOWN ");
		valid = true;
	}
	if (state & SNES_LEFT) { 
		printf("LEFT ");
		valid = true;
	}
	if (state & SNES_RIGHT) { 
		printf("RIGHT ");
		valid = true;
	}
	if (state & SNES_A) { 
		printf("A ");
		valid = true;
	}
	if (state & SNES_X) { 
		printf("X ");
		valid = true;
	}
	if (state & SNES_L) { 
		printf("L ");
		valid = true;
	}
	if (state & SNES_R) { 
		printf("R ");
		valid = true;
	}
	if (valid){ printf("\n"); }
	return valid;
}
