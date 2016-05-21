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
#define DEBUG 0

int state = 0;
uint8_t SNES_MESSAGE_BUFFER[SNESMessage_size];

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.


void setup(){

	Serial.begin(9600);
	printf_begin();
	
	// Setup and configure rf radio
	radio.begin();
	
	//explicitly enable auto ack
	radio.setAutoAck(true); 

	//the radio amplification should be minimal since the range is short
	radio.setPALevel(RF24_PA_MIN);

	//since we're just transmitting gamepad data no need to get fancy with bitrates
	radio.setDataRate(RF24_250KBPS);

	//no need to check crc because we're not going to retransmit
	radio.setCRCLength(RF24_CRC_DISABLED);

	//payload will be the protobuf object size
	radio.setPayloadSize(SNESMessage_size);
	
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);	
	if(DEBUG) radio.printDetails();   
	setup_controller();
}

void loop(void) {
	
	//Called when STANDBY-I mode is engaged (User is finished sending)		
	if (buildMessage() > 0){
		if (!radio.write(&SNES_MESSAGE_BUFFER, SNESMessage_size)){
			Serial.println(F("failed."));
		}		
		if (DEBUG){
			Serial.print("TX: ");
			for (int i = 0; i < SNESMessage_size; i++) { Serial.print(SNES_MESSAGE_BUFFER[i]);; Serial.print(" "); }
			Serial.println("");
		}
	}
}


//write out currently pressed buttons during message encoding
//TODO: safety here when more than 3 buttons pressed!
//void set_pressed_buttons(SNESMessage* message){ //SNESMessage_ControllerButton* pressedButtons){
//	int idx = 0;
//	SNESMessage_ControllerButton* pressedButtons = message->pressedButtons;
//	state = buttons();
//		
//	if (state & SNES_B)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_B_BUTTON;
//	if (state & SNES_Y)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_Y_BUTTON;
//	if (state & SNES_SELECT) 
//		pressedButtons[idx++] = SNESMessage_ControllerButton_SELECT_BUTTON;
//	if (state & SNES_START)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_START_BUTTON;
//	if (state & SNES_UP)     
//		pressedButtons[idx++] = SNESMessage_ControllerButton_UP_BUTTON;
//	if (state & SNES_DOWN)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_DOWN_BUTTON;
//	if (state & SNES_LEFT)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_LEFT_BUTTON;
//	if (state & SNES_RIGHT)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_RIGHT_BUTTON;
//	if (state & SNES_A)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_A_BUTTON;
//	if (state & SNES_X)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_X_BUTTON;			
//	if (state & SNES_L)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_L_BUTTON;
//	if (state & SNES_R)
//		pressedButtons[idx++] = SNESMessage_ControllerButton_R_BUTTON;		
//	message->pressedButtons_count = idx;
//}


int buildMessage(){
	int message_length = -1;
	bool status = false;	
	SNESMessage message = SNESMessage_init_zero;
	/*message.voltage = 1.0;
	message.has_voltage = true;*/
	message.buttonRegister = buttons();

	//set_pressed_buttons(&message);	
	pb_ostream_t stream = pb_ostream_from_buffer(SNES_MESSAGE_BUFFER, SNESMessage_size);
	status = pb_encode(&stream, SNESMessage_fields, &message);
	message_length = stream.bytes_written;
	
	if (!status){
		printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
		return NULL;
	}
	
	
	return message_length;
}

