/*
  AD7327.cpp  Version 0.1
  Analog Devices AD7327 SPI ADC Class for Arduino
  Created by Nate Gemelke
  May, 2014

*/

#include <SPI.h>                 // Arduino IDE SPI library - uses AVR hardware SPI features
#include <AD7327.h>            // Header files for this class
#include <Arduino.h>

// Here we have things for the SPI bus configuration

#define    CLOCK_DIVIDER (2)           // SPI bus speed to be 1/2 of the processor clock speed - 8MHz on most Arduinos

// Constructor to instantiate an instance of MCP to a specific chip (address)

AD7327::AD7327(void) {
  pinMode(CS_ADC,OUTPUT);                   // set the slave-select Arduino pin as an output
  digitalWrite(CS_ADC,HIGH);                  // Set SS pin high
  //PORTD |= 0b01000000;					// Take the slave-select pin HIGH to avoid conflicts
}

// GENERIC WORD WRITE - will write a word to a register pair, LSB to first register, MSB to next higher value register, returns data retrieved from MISO

uint16_t AD7327::wordWrite(uint16_t writedata) {  // Accept the start register and word
  uint8_t lb;
  uint8_t hb;
  digitalWrite(CS_ADC,LOW);                  // Set SS pin high
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE2);//Ethernet must be SPI_MODE0(?)
  //PORTD &= 0b10111111;       // Direct port manipulation speeds taking Slave Select LOW before SPI action
  hb=SPI.transfer((uint8_t) (writedata >> 8));              // Shift the high byte down to the low byte location and send
  lb=SPI.transfer((uint8_t) (writedata));
  digitalWrite(CS_ADC,HIGH);                  // Set SS pin high
  //PORTD |= 0b01000000;                                 // Direct port manipulation speeds taking Slave Select HIGH after SPI action
  return word(hb,lb);
}

// READ FUNCTIONS - BY FULL CHANNEL SCAN or Single Channel Read

void AD7327::analogRead(void) {                // This function will read all 8 channels of ADC, and save them as a 8x2-byte in the format 0x(CCCSHHHH)(LLLLLLLL) x 16 to _data of object
  uint16_t sample;
  AD7327::wordWrite(0b1001110000111000);                 // setup for autosequencing through all inputs
  for (int j=0;j<8;j++){                       // Initialize a variable to hold the read values to be returned
	sample=AD7327::wordWrite(0x00);      // retrieve a sample - comes back from chip in format CCCSHHHHLLLLLLLL. C-channel, S-sign
	//Serial.println((int) ((sample & 0b1110000000000000) >> 13));
	_data[(int) ((sample & 0b1110000000000000) >> 13)] = sample;

  }
}

uint16_t AD7327::analogRead(uint8_t chan) {    // This function will read one channel of ADC, and return it as an integer in the range 0 to 8192
  uint16_t sample;
  wordWrite(0b1000000000110000 | (chan << 13));			// setup for sample on channel chan
  sample=wordWrite(0b1000000000110000 | (chan << 13));  // retrieve a sample
  return (uint16_t) (sample & 0b0001111111111111);
}
