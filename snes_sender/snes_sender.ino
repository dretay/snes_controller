/*TODO
- should only be able to send
- protobuf class
- controller hardware power efficiency
- wake on interrupt?
*/
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "snes_controller.h"

int state = 0;

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  

typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

// A single byte to keep track of the data being sent back and forth
byte counter = 1;


void setup(){

	Serial.begin(9600);
	printf_begin();
	
	// Setup and configure rf radio
	radio.begin();
	//do not ack - controller will be fire and forget
	radio.setAutoAck(false); 

	//the radio amplification should be minimal since the range is short
	radio.setPALevel(RF24_PA_MIN);

	//since we're just transmitting directional data no need to get fancy with bitrates
	radio.setDataRate(RF24_250KBPS);

	//no need to check crc because we're not going to retransmit
	radio.setCRCLength(RF24_CRC_DISABLED);

	//TODO: this should be infered from the proto object
	radio.setPayloadSize(1);                
	
	radio.openWritingPipe(pipes[0]);        // Both radios listen on the same pipes by default, and switch when writing
	radio.openReadingPipe(1, pipes[1]);	
	radio.printDetails();                   // Dump the configuration of the rf unit for debugging
	counter = 69;

	setup_controller();
}

void loop(void) {

	printf("Now sending %d as payload. ", counter);
	byte gotByte;
	unsigned long time = micros();                          // Take the time, and send it.  This will block until complete   
	//Called when STANDBY-I mode is engaged (User is finished sending)
	if (!radio.write(&counter, 1)){
		Serial.println(F("failed."));
	}
	state = buttons();
	if (state & SNES_B)      Serial.print("B");
	if (state & SNES_Y)      Serial.print("Y");
	if (state & SNES_SELECT) Serial.print("select");
	if (state & SNES_START)  Serial.print("start");
	if (state & SNES_UP)     Serial.print("up");
	if (state & SNES_DOWN)   Serial.print("down");
	if (state & SNES_LEFT)   Serial.print("left");
	if (state & SNES_RIGHT)  Serial.print("right");
	if (state & SNES_A)      Serial.print("a");
	if (state & SNES_X)      Serial.print("x");
	if (state & SNES_L)      Serial.print("l");
	if (state & SNES_R)      Serial.print("r");
	Serial.println();

	// Try again later
	delay(1000);
	

	
}
