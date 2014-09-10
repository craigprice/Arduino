/*
  AD7327.h  Version 0.1
  Analog AD7327 SPI ADC Class for Arduino
  Created by Nate Gemelke
  May, 2014

*/

#ifndef AD7327_h
#define AD7327_h


const static int CS_ADC = 6;    // SPI bus slave select output to pin 10 - READ ARDUINO SPI DOCS BEFORE CHANGING!!!  -  THIS CONSTANT IS NOT USED!! THE PORTD/B manipulations implement the chip-select line!!  NDG

//Default input range is +/- 10V
//From page 25 of the datasheet, shows the control/sequence operation.

//Control Register first byte
const static uint8_t    CON_REG_ADC          = 0b10000000;  //Select the Control Register
const static uint8_t    CHANNEL_ADDR_ADC     = 0b00011100;  //Channel Address Bits. Used in combination with CHANNEL_SEQ_ADC. All channels will be read from
const static uint8_t    INPUT_MODE_ADC       = 0b00000000;  //Single Ended Inputs w/ respect to ground

//Control Register second byte
const static uint8_t    POW_MAN_ADC          = 0b00000000;  //Pins 7,6: Normal Power Operation
const static uint8_t    CODING_ADC           = 0b00100000;  //Pins 5: Straight Binary Coding
const static uint8_t    ENABLE_INT_REF_ADC   = 0b00010000;  //Pins 4: Enable Internal Reference
const static uint8_t    CHANNEL_SEQ_ADC      = 0b00001000;  //Pins 3,2: Enable Channel Sequencer from channel 0 to channel input in the address. Used in conjunction with CHANNEL_ADDR_ADC

//Sequence Register First byte
const static uint8_t    SEQ_REG_ADC          = 0b11100000;  //Select the Sequence Register


class AD7327 {
  public:
    AD7327(void);                               // Constructor to instantiate a discrete IC as an object
    uint16_t wordWrite(unsigned int);   // Typically only used internally, but allows the user to write any register pair if needed, so it's public
    void analogRead(void);                   // Reads all 8 input pins into _data
    uint16_t analogRead(uint8_t);                   // Reads one input pin and returns value as integer
	uint16_t _data[8];                       // Caches the readings from all 8 channels in the form CCCSHHHHLLLLLLLL
  private:
	uint16_t dummy;
 };

#endif //AD7327
