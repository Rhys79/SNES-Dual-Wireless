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
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

//
// Hardware Configuration
//

RF24 radio(9,10);

//
// Variable Inits
//

volatile unsigned long state2 = 0xFFFFFFFF;
volatile byte i = 0;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
volatile int strobe = 2;
int clock = 3;
volatile int data1 = 5;
volatile int data2 = 6;
bool firstLoop = true;
volatile int status2 = 1;

void setup()
{
  //Setup radio and serial debugging
  radio.begin();
  radio.setRetries(0,15);
  radio.enableDynamicPayloads();
  Serial.begin(57600);
  
  //setup SNES pins
  pinMode(strobe, INPUT);
  pinMode(clock, INPUT);
  pinMode(data1, OUTPUT); digitalWrite(data1, LOW);
  pinMode(data2, OUTPUT); digitalWrite(data2, LOW);

  //open radio pipes
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  //Dump the configuration of the RF unit for debugging - remove in final code
  radio.printDetails();

  //Setup Interupts  
  attachInterrupt(strobe,latch,CHANGE);
  attachInterrupt(clock,data,RISING);

}

void loop()
{
  //check if this is the first execution of loop()
  if (firstLoop) {
    int status1 = 1;
    
    //send ping packet to let transmitter know we are ready for a packet
    bool ok = radio.write( &status1, sizeof(int));
    
    //begin listenting for button state packet
    radio.startListening();
    
    //debug check to make sure ping packet was sent - remove in final code
    if (!ok) {
      Serial.println("sync packet transmission failed");
    }
    else {
      Serial.println("sync packet transmission successful");
           
      //let the program know we have successfully executed the first loop
      firstLoop = false;
    }
  }
  
  //check for data packet from transmitter
  if ( radio.available() )
  {
    //read data packet from transmitter
    unsigned long state = 0;
    radio.read( &state, sizeof(unsigned long) );
    
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
    
    //set first bit on data lines
    digitalWrite(data1,bitRead(state2,i));
    digitalWrite(data2,bitRead(state2,(i+16)));
    
    //initialize clock signal counter to 0
    i = 0;
    
    //debug status output
    Serial.println("Bit0 out");
  }
}

//Data interrupt routine

void data()
{
  //increment clock signal counter
  i++;
  
  //output next bit on data lines
  digitalWrite(data1,bitRead(state2,i));
  digitalWrite(data2,bitRead(state2,(i+16)));
  
  //debug status output
  Serial.print("Bit");
  Serial.print(i);
  Serial.println(" out");
  
  //check to see if this is the final clock cycle in a read cycle
  if(i=15)
  {
    //drive data lines low
    digitalWrite(data1,LOW);
    digitalWrite(data2,LOW);
    
    //take radio out of listening mode
    radio.stopListening();
    
    //read ping bit into ISR local variable from volatile global
    int status1 = status2;
    
    //send ping packet to transmitter
    bool ok = radio.write( &status1, sizeof(int));
    
    //debug check to see if packet sent - remove in final code 
    if (!ok) {
      Serial.println("sync packet transmission failed");
    }
    else {
      Serial.println("sync packet transmission successful");
    }
    
    //put radio back in listening mode
    radio.startListening();
  }
}
