/*TODO
- controller hardware power efficiency
- wake on interrupt?
- does a higher data rate help w/ dropped? need stats to confirm
- only send a message if there is a change
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
#define DEBUG 1
#define MAX_RETRY  5
int LAST_CONTROLLER_READING =0;
uint8_t SNES_MESSAGE_BUFFER[SNESMessage_size];

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };            


void setup(){

	Serial.begin(115200);
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
	int retry_cnt = 0;
	if (buildMessage() > 0){

		while (!radio.write(&SNES_MESSAGE_BUFFER, SNESMessage_size) && retry_cnt++ < MAX_RETRY){			
		}		
		if (DEBUG){
			if (retry_cnt < MAX_RETRY){
				printf("TX (%d): ", retry_cnt);
			}
			else{
				printf("ABORTING TX !!! ");
			}
			for (int i = 0; i < SNESMessage_size; i++) { Serial.print(SNES_MESSAGE_BUFFER[i]);; Serial.print(" "); }
			printf("\n");
		}
	}
}


int buildMessage(){
	int message_length = -1;
	int current_button_reading = 0;
	bool status = false;	
	SNESMessage message = SNESMessage_init_zero;
	/*message.voltage = 1.0;
	message.has_voltage = true;*/
	current_button_reading = buttons();
	
	if (current_button_reading == LAST_CONTROLLER_READING)
		return 0;

	
	message.buttonRegister = current_button_reading;

	//set_pressed_buttons(&message);	
	pb_ostream_t stream = pb_ostream_from_buffer(SNES_MESSAGE_BUFFER, SNESMessage_size);
	status = pb_encode(&stream, SNESMessage_fields, &message);
	message_length = stream.bytes_written;
	
	if (!status){
		printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
		return 0;
	}

	LAST_CONTROLLER_READING = current_button_reading;
	
	
	return message_length;
}

