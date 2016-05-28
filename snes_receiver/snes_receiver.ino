

/*TODO
- on error give statistics of fail / success counts
- possible to get signal strength?
*/
#include <pb.h>
#include <pb_decode.h>
#include <pb_common.h>

#include <SPI.h>
#include <Joystick.h>
#include <limits.h>
#include "printf.h"
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "simple.pb.h"
#include "snes_bitmasks.h"
#define DEBUG 0

//#define STAT_WINDOW 4096
#define STAT_WINDOW 128

// Hardware configuration
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };           

//RF message buffer
byte SNES_MESSAGE_BUFFER[SNESMessage_size];

//store STAT_WINDOW messages - 1 = success 0 = failure
typedef unsigned long bfield_t[STAT_WINDOW/sizeof(long) ];

bfield_t MESSAGE_STATS;

unsigned short MESSAGE_COUNTER = 0;


void setup(){

	Serial.begin(115200);
	
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

  Joystick.begin();
  printf_begin();

}
void apply_pressed_buttons(SNESMessage* message){ 
  int state = message->buttonRegister;

  if (!((state & SNES_UP) || (state & SNES_DOWN)))
    Joystick.setYAxis(0);
  if (!((state & SNES_LEFT) || (state & SNES_RIGHT)))
    Joystick.setXAxis(0);
    
  if (state & SNES_B)
    Joystick.pressButton(0);
  else
    Joystick.releaseButton(0);
    
  if (state & SNES_Y)
    Joystick.pressButton(1);
  else
    Joystick.releaseButton(1);
    
  if (state & SNES_SELECT) 
    Joystick.pressButton(2);
 else
    Joystick.releaseButton(2);
    
  if (state & SNES_START)
    Joystick.pressButton(3);
  else
    Joystick.releaseButton(3);
    
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
  else
    Joystick.releaseButton(4);
    
  if (state & SNES_X)
    Joystick.pressButton(5);
  else
    Joystick.releaseButton(5);
    
  if (state & SNES_L)
    Joystick.pressButton(6);
  else
    Joystick.releaseButton(6);
    
  if (state & SNES_R)
    Joystick.pressButton(7);
  else
    Joystick.releaseButton(7);
    

}
int CURR_STAT_IDX = 0;
int CURR_STAT_BIT = 0;

//generate a summary of the current statistics
void print_stats(){
  int stat_outer = 0;
  int stat_inner = 0;  
  int stat_failure = 0;
  long stat_curr;     
  
  for(stat_outer = 0;stat_outer < (STAT_WINDOW/sizeof(long)); stat_outer++){
    stat_curr = MESSAGE_STATS[stat_outer];      
    for(stat_inner = 0;stat_inner < (8 * sizeof(long)); stat_inner++){
      if(!(stat_curr & 0x01)){        
        stat_failure++;
      }
      stat_curr = stat_curr >> 1;
    }
  }
  printf("%d of last %d messages were invalid\n", stat_failure, (STAT_WINDOW*2*sizeof(long)));  
  printf("==============================================");
}

//record a single stat data point
//use some bit shifting to try to make this fast since its going to delay controller response 
//todo: should this be a debug feature?
void log_stat(bool success){
  if(CURR_STAT_IDX <= (STAT_WINDOW/sizeof(long)) ){
    if(CURR_STAT_BIT <= (8 * sizeof(long))){
      if(success) 
        MESSAGE_STATS[CURR_STAT_IDX] |= 1 << CURR_STAT_BIT;
      else 
        MESSAGE_STATS[CURR_STAT_IDX] &= ~(1<<CURR_STAT_BIT); 
      
      CURR_STAT_BIT++;     
    }else{   
      CURR_STAT_BIT = 0;
      CURR_STAT_IDX++;
      log_stat(success);
    }
  }else{
    //time to print out the stats and reset
    CURR_STAT_IDX = 0;
    CURR_STAT_BIT = 0;
    print_stats();
    log_stat(success);    
  }
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
    log_stat(status);
    if (!status){
      Serial.print("Decoding failed : ");
      Serial.println(PB_GET_ERROR(&stream));
    }
    else{
      apply_pressed_buttons(&message);
    }


	}
}
