/*TODO
- should only be able to send
- protobuf class
- controller hardware power efficiency
- wake on interrupt?
*/
#include <pb.h>
#include <pb_encode.h>
#include <pb_common.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "snes_controller.h"
#include "simple.pb.h"

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
SNESMessage snesMessage;

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
	
	
	//Called when STANDBY-I mode is engaged (User is finished sending)
	if (!radio.write(&counter, 1)){
		Serial.println(F("failed."));
	}

	// Try again later
	delay(1000);	
}

bool add_button_to_stream(pb_ostream_t *stream, const pb_field_t *field, SNESMessage_ControllerButton button){
	if (!pb_encode_tag_for_field(stream, field))
		return false;

	if (!pb_encode_varint(stream, button))
		return false;

	return true;
}
//write out currently pressed buttons during message encoding
bool list_button_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg){
	state = buttons();
	bool error = true;
	
	if (state & SNES_B)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_B_BUTTON);
	
	if (state & SNES_Y)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_Y_BUTTON);
	
	if (state & SNES_SELECT) 
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_SELECT_BUTTON);

	if (state & SNES_START)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_START_BUTTON);
	
	if (state & SNES_UP)     
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_UP_BUTTON);
	
	if (state & SNES_DOWN)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_DOWN_BUTTON);
	
	if (state & SNES_LEFT)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_LEFT_BUTTON);
	
	if (state & SNES_RIGHT)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_RIGHT_BUTTON);
	
	if (state & SNES_A)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_A_BUTTON);
	
	if (state & SNES_X)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_X_BUTTON);
	
	if (state & SNES_L)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_L_BUTTON);
	
	if (state & SNES_R)
		error |= add_button_to_stream(stream, field, SNESMessage_ControllerButton_R_BUTTON);
	return error;
}


//void sendData(){
//	bool status;
//	size_t message_length;
//	uint8_t buffer[SNESMessage_size];
//	snesMessage = SNESMessage_init_zero;
//	snesMessage.controllerButton.funcs.encode = &list_button_callback;
//
//
//	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
//	/*message.lucky_number = inputMagicNumber + 1;
//	Serial.println("Sending back: ");
//	Serial.println(message.lucky_number);
//	status = pb_encode(&stream, SimpleMessage_fields, &message);
//	message_length = stream.bytes_written;*/
//
//	if (!status){
//		printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
//	}
//}

