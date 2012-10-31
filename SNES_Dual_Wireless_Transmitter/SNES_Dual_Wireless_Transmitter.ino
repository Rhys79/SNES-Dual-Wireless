/*
Copyright (C) 2012 John Byers <jbyers2@wgu.edu>
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation
*/

/**
 * Dual Wireless Retro Controller Adapter Transmitter
 *
 * This is the transmitter side code for the adapter set.
 */

#include "SNESpadDual.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24_config.h"
#include "printf.h"
#include "snes.h"

// Hardware Configuration
RF24 radio(9,10);
SNESpad nintendo = SNESpad(4,3,5,6);

//
//Variable Inits
//

buttons_t state = 0;

void setup()
{
  
  //setup radio and serial debugging lines
  radio.begin();
  radio.setRetries(0,15);
  radio.enableDynamicPayloads();
  Serial.begin(57600);
  printf_begin();

  //setup radio pipes and put radio in listening mode
  radio.openWritingPipe(PIPE1);
  radio.openReadingPipe(1,PIPE2);
  radio.startListening();
  
  //Dump the configuration of the RF unit for debugging
  radio.printDetails();
}

void loop() {
  
  //check if radio has recieved ping from reciever and read data in
  ping_t ping = 0;
  if (radio.available() ) {
    radio.read (&ping, sizeof(ping_t));
    Serial.println("ping recieved");
  }
  
  //if ping has been recieved, read in button states and transmit
  if (ping) {
    
    //take radio out of listening mode
    radio.stopListening();
    
    // Get controller button states 
    state = nintendo.buttons();
    
    //debugging output - remove in final code
    Serial.println(~state, BIN);
  
    // Send button states to reciever
    bool ok = radio.write( &state, sizeof(buttons_t) );
    
    //debugging output - remove in final code
    if (ok) {
      Serial.println("transmission successful");
    }
    else {
      Serial.println("transmission failed!!");
    }
   
    //put radio back in listening mode to await next ping
    radio.startListening();
  }
  else {
    Serial.println("ping not recieved");
  }
}
