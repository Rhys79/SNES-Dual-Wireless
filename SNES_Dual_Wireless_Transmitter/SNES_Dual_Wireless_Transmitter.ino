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

#include <SNESpadDual.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

// Hardware Configuration
RF24 radio(9,10);
SNESpad nintendo = SNESpad(4,3,5,6);

// Variable Inits
unsigned long state = 0;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup()
{
  
  //setup radio and serial debugging lines
  radio.begin();
  radio.setRetries(0,15);
  radio.enableDynamicPayloads();
  Serial.begin(57600);

  //setup radio pipes and put radio in listening mode
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  
  //Dump the configuration of the RF unit for debugging
  radio.printDetails();
}

void loop() {
  
  //check if radio has recieved ping from reciever and read data in
  int ready = 0;
  if (radio.available() ) {
    radio.read (&ready, sizeof(char));
  }
  
  //if ping has been recieved, read in button states and transmit
  if (ready) {
    
    //take radio out of listening mode
    radio.stopListening();
    
    // Get controller button states 
    state = nintendo.buttons();
    
    //debugging output - remove in final code
    Serial.println(~state, BIN);
  
    // Send button states to reciever
    bool ok = radio.write( &state, sizeof(unsigned long) );
    
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
}
