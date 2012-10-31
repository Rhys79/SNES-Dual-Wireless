/*
Copyright (C) 2012 John Byers <jbyers2@wgu.edu>
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation
*/

/** 
 * Dual Wireless Retro Controller Adapter Transmitter
 *
 * This is the reciever side code for the adapter set.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24_config.h"
#include "snes.h"
#include "printf.h"
#include "snes.h"
#define strobe 2
#define clock 3
#define data1 5
#define data2 6

//
// Hardware Configuration
//

RF24 radio(9,10);

//
// Variable Inits
//

volatile unsigned long state2 = 0xFFFFFFFF;
volatile byte loopCount = 0;
bool firstLoop = true;

void setup()
{
  //Setup radio and serial debugging
  radio.begin();
  radio.setRetries(0,15);
  radio.enableDynamicPayloads();
  Serial.begin(57600);
  printf_begin();
  
  //setup SNES pins
  pinMode(strobe, INPUT);
  pinMode(clock, INPUT);
  pinMode(data1, OUTPUT); digitalWrite(data1, LOW);
  pinMode(data2, OUTPUT); digitalWrite(data2, LOW);

  //open radio pipes
  radio.openWritingPipe(PIPE2);
  radio.openReadingPipe(1,PIPE1);
  
  //Dump the configuration of the RF unit for debugging - remove in final code
  radio.printDetails();

  //Setup Interupts  
  attachInterrupt(strobe,latch,CHANGE);
  attachInterrupt(clock,data,RISING);

}

void loop()
{
  //check if this is the first execution of loop()
  while (firstLoop) {
    
    //send ping packet to let transmitter know we are ready for a packet
    ping_t ping = PING_VALUE;
    bool ok = radio.write( &ping, sizeof(ping_t) );
    
    //debug check to make sure ping packet was sent - remove in final code
    if (!ok) {
      Serial.println("sync packet transmission failed");
    }
    else {
      Serial.println("sync packet transmission successful");
           
      //let the program know we have successfully executed the first loop
      firstLoop = false;
      
      //begin listenting for button state packet
      radio.startListening();
    }
  }
  
  //check for data packet from transmitter
  if ( radio.available() )
  {
    //read data packet from transmitter
    buttons_t state = 0;
    radio.read( &state, sizeof(buttons_t) );
    
    //debug output recieved packet contents - remove in final code
    Serial.println(state, BIN);
    
    //copy button state data to volatile variable for use in interrupts
    state2 = state;
  }
  
  //debuging output if no data packet present yet  - remove in final code
  else
  {
    Serial.println("No data recieved yet");
  }
}

//Latch interrupt routine

void latch()
{
  //check to see if latch signal is high or low
  if (strobe) {
    
    //set both data lines high
    digitalWrite(data1,HIGH);
    digitalWrite(data2,HIGH);
  }
  else {

    //initialize clock signal counter to 0
    loopCount = 0;
    
    sendNextBits();
    
    //debug status output
    Serial.println("Bit0 out");
  }
}

//Data interrupt routine

void data()
{
  //increment clock signal counter
  loopCount++;
  
  sendNextBits();
  
  //debug status output
  Serial.print("Bit");
  Serial.print(loopCount);
  Serial.println(" out");
  
  //check to see if this is the final clock cycle in a read cycle
  if(15==loopCount)
  {
    //drive data lines low
    digitalWrite(data1,LOW);
    digitalWrite(data2,LOW);
    
    //take radio out of listening mode
    radio.stopListening();
    
    //send ping packet to transmitter 
    bool ok = false;
    while (!ok) {
      ping_t ping = PING_VALUE;
      bool ok = radio.write( &ping, sizeof(ping_t) );
      if (!ok) {
        Serial.println("sync packet transmission failed");  //debug check to see if packet sent - remove in final code
      }
    }
    
    Serial.println("sync packet transmission successful");
    
    //put radio back in listening mode
    radio.startListening();
  }
}

void sendNextBits()
{
  
  //output next bit on data lines   
  digitalWrite(data1,bitRead(state2,loopCount));
  digitalWrite(data2,bitRead(state2,(loopCount+16)));
}
