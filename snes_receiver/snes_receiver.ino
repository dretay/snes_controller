

/*TODO
- should only be able to recieve
- protobuf class
- controller hardware power efficiency
- wake on interrupt?
*/
#include <pb.h>
#include <pb_decode.h>
#include <pb_common.h>

#include <SPI.h>
#include <Joystick.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "simple.pb.h"
#include "snes_bitmasks.h"
#define DEBUG 0

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };           

byte SNES_MESSAGE_BUFFER[SNESMessage_size];

// A single byte to keep track of the data being sent back and forth
byte counter = 1;

void setup(){

	Serial.begin(9600);
	
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

	radio.openWritingPipe(pipes[1]);       
	radio.openReadingPipe(1, pipes[0]);
	radio.startListening();                
  if(DEBUG) radio.printDetails();   

  Joystick.begin();

}
void apply_pressed_buttons(SNESMessage* message){ 
  int state = message->buttonRegister;
    
  if (state & SNES_B)
    Joystick.pressButton(0);
    
  if (state & SNES_Y)
  Joystick.pressButton(1);
    
  if (state & SNES_SELECT) 
    Joystick.pressButton(2);
    
  if (state & SNES_START)
    Joystick.pressButton(3);
    
  if (state & SNES_UP)     
    Joystick.setYAxis(127);
    
  if (state & SNES_DOWN)
    Joystick.setYAxis(-127);
    
  if (state & SNES_LEFT)
    Joystick.setXAxis(-127);
    
  if (state & SNES_RIGHT)
    Joystick.setXAxis(127);
    
  if (state & SNES_A)
    Joystick.pressButton(4);
    
  if (state & SNES_X)
    Joystick.pressButton(5);
    
  if (state & SNES_L)
    Joystick.pressButton(6);
    
  if (state & SNES_R)
    Joystick.pressButton(7);
    

}
void loop(void) {

	byte pipeNo;
  bool status = false;  
  SNESMessage message = SNESMessage_init_zero;
	while (radio.available(&pipeNo)){		
		radio.read(&SNES_MESSAGE_BUFFER, SNESMessage_size);
		if (DEBUG){
      Serial.print("RX: ");
      for (int i = 0; i < SNESMessage_size; i++) { Serial.print(SNES_MESSAGE_BUFFER[i]);; Serial.print(" "); }
      Serial.println("");
    }    
    
    pb_istream_t stream = pb_istream_from_buffer(SNES_MESSAGE_BUFFER, SNESMessage_size);
    status = pb_decode(&stream, SNESMessage_fields, &message);
    if (!status){
      Serial.print("Decoding failed : ");
      Serial.println(PB_GET_ERROR(&stream));
    }
    else{
      apply_pressed_buttons(&message);
    }    
	}
}
