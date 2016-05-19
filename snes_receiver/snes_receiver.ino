/*TODO
- should only be able to recieve
- protobuf class
- controller hardware power efficiency
- wake on interrupt?
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

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
	Serial.print(F("\n\rRF24/examples/pingpair_ack/\n\rROLE: "));
	Serial.println(role_friendly_name[role]);
	Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

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

	radio.openWritingPipe(pipes[1]);        // Both radios listen on the same pipes by default, and switch when writing
	radio.openReadingPipe(1, pipes[0]);
	radio.startListening();                 // Start listening
	radio.printDetails();                   // Dump the configuration of the rf unit for debugging

}

void loop(void) {

	byte pipeNo;
	byte gotByte;
	while (radio.available(&pipeNo)){
		radio.read(&gotByte, 1);
		printf("Got response %d\n\r", gotByte);

	}



}