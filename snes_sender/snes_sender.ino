/*TODO
- controller hardware power efficiency
- wake on interrupt?
- does a higher data rate help w/ dropped? need stats to confirm
- are 2 pipes necessary for message acks?
- generalize stat library out of receiver and add it here
- w/ radio off still draws 2ma - need to switch it off w/ a fet
- sleep strategy for arduino
- lower clock rate for arduino
- should only tx when the controller code is valid
- (should we reset when we're getting abunch of garbage?)
*/
#include <LowPower.h>
#include <pb.h>
#include <pb_encode.h>
#include <pb_common.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "snes_controller.h"
#include "simple.pb.h"
#include <avr/power.h>


#define DEBUG 1
#define MAX_RETRY  10



#define MS_PER_SEC  (1000UL)
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

#define FIVE_MINUTES_MS (5 * SECS_PER_MIN * MS_PER_SEC )

int LAST_CONTROLLER_READING =0;
uint8_t SNES_MESSAGE_BUFFER[SNESMessage_size];

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };      

// Duplicate key press cnt
unsigned long int NO_ACTIVITY_START;


void setup(){

	Serial.begin(57600);
	printf_begin();
	
	// Setup and configure rf radio
	radio.begin();
	
	//explicitly enable auto ack
	radio.setAutoAck(true); 

	//the radio amplification should be minimal since the range is short
	radio.setPALevel(RF24_PA_LOW);

	//since we're just transmitting gamepad data no need to get fancy with bitrates
	radio.setDataRate(RF24_250KBPS);

	//no need to check crc because we're not going to retransmit
	radio.setCRCLength(RF24_CRC_8);

	//payload will be the protobuf object size
	radio.setPayloadSize(SNESMessage_size);
	
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);	
	if(DEBUG) radio.printDetails();   
	setup_controller();

	//disable unused perhipherals 
	power_adc_disable();
	power_twi_disable();
	
}

void loop(void) {	
	int retry_cnt = 0;	
	if (buildMessage() > 0){
		NO_ACTIVITY_START = 0;
		while (!radio.write(&SNES_MESSAGE_BUFFER, SNESMessage_size) && retry_cnt++ < MAX_RETRY){			
		}		
		if (DEBUG){
			if (retry_cnt < MAX_RETRY){
				printf("TX(%d): ", retry_cnt);
				for (int i = 0; i < SNESMessage_size; i++) { Serial.print(SNES_MESSAGE_BUFFER[i]);; Serial.print(" "); }
				printf("\n");
			}
			else{
				printf("ABORTING!!! \n");
			}
		}
	}	
	else{
		if (NO_ACTIVITY_START == 0){			
			NO_ACTIVITY_START = millis();			
		}
		else if ((millis() - NO_ACTIVITY_START) > FIVE_MINUTES_MS){			
			//set the no activity to be now - 8 seconds so that when controller wakes up it will poll a little before sleeping again
			NO_ACTIVITY_START = millis() - FIVE_MINUTES_MS + (MS_PER_SEC * 4);
			if (DEBUG){
				printf("sleeping for 4s\n");
				delay(100);
			}			
			//adc is disabled in setup - don't touch it here
			LowPower.powerDown(SLEEP_4S, ADC_ON, BOD_OFF);
			printf("waking up for 4s\n");
			delay(500);
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
	
	if (current_button_reading == LAST_CONTROLLER_READING){
		return 0;
	}	
	
	message.buttonRegister = current_button_reading;

	//clear out the message buffer only when we're going to send a new message
	memset(SNES_MESSAGE_BUFFER, 0, sizeof SNES_MESSAGE_BUFFER);
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

