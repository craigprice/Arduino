/*
  MCP23S17.cpp  Version 0.1
  Microchip MCP23S17 SPI I/O Expander Class for Arduino
  Created by Cort Buffington & Keith Neufeld
  March, 2011

  Features Implemented (by word and bit):
    I/O Direction
    Pull-up on/off
    Input inversion
    Output write
    Input read

  Interrupt features are not implemented in this version
  byte based (portA, portB) functions are not implemented in this version

  NOTE:  Addresses below are only valid when IOCON.BANK=0 (register addressing mode)
         This means one of the control register values can change register addresses!
         The default values is 0, so that's how we're using it.

         All registers except ICON (0xA and 0xB) are paired as A/B for each 8-bit GPIO port.
         Comments identify the port's name, and notes on how it is used.

         *THIS CLASS ENABLES THE ADDRESS PINS ON ALL CHIPS ON THE BUS WHEN THE FIRST CHIP OBJECT IS INSTANTIATED!

  USAGE: All Read/Write functions except wordWrite are implemented in two different ways.
         Individual pin values are set by referencing "pin #" and On/Off, Input/Output or High/Low where
         portA represents pins 0-7 and portB 8-15. So to set the most significant bit of portB, set pin # 15.
         To Read/Write the values for the entire chip at once, a word mode is supported buy passing a
         single argument to the function as 0x(portB)(portA). I/O mode Output is represented by 0.
         The wordWrite function was to be used internally, but was made public for advanced users to have
         direct and more efficient control by writing a value to a specific register pair.
*/

#include <SPI.h>                 // Arduino IDE SPI library - uses AVR hardware SPI features
#include <MCP23S17.h>            // Header files for this class
#include <Arduino.h>

// Constructor to instantiate an instance of MCP to a specific chip (address)

MCP::MCP(void) {
  MCP::address     = 0x00;
  MCP::modeCache   = 0xFFFF;                  // Default I/O mode is all input, 0xFFFF
  MCP::outputCache = 0x0000;                  // Default output state is all off, 0x0000
  MCP::pullupCache = 0x0000;                  // Default pull-up state is all off, 0x0000
  MCP::invertCache = 0x0000;                  // Default input inversion state is not inverted, 0x0000
  SPI.begin();                                // Start up the SPI bus… crank'er up Charlie!
  MCP::_openCommLink();
  MCP::byteWrite(IOCON_MCP, ADDR_ENABLE_MCP); //Set the default chip control registers. IOCON.BANK =0, SPI sequential R/W, hardware enabled, etc.
  //MCP::wordWrite(IODIRA_MCP, 0x0000);
  //MCP::wordWrite(IPOLA_MCP, 0x0000);
  //MCP::wordWrite(GPINTENA_MCP, 0x0000);
  //MCP::wordWrite(DEFVALA_MCP, 0x0000);
  //MCP::wordWrite(INTCONA_MCP, 0x0000);
  //MCP::wordWrite(GPPUA_MCP, 0x0000);
  //MCP::wordWrite(OLATA_MCP, 0x0000);
  pinMode(CS_MCP,OUTPUT);                     // set the slave-select Arduino pin as an output. May need this in the setup function as well.
  digitalWrite(CS_MCP,HIGH);                  // Set SS pin high
};

void MCP::_openCommLink(void){
  SPI.setClockDivider(CLOCK_DIVIDER_MCP);     // Sets the SPI bus speed
  SPI.setBitOrder(MSBFIRST);                  // Sets SPI bus bit order
  SPI.setDataMode(SPI_MODE0);                 // Sets the SPI bus timing mode
  //digitalWrite(CS_MCP,LOW);                  // Set SS pin high
  PORTB &= 0b11111101;                        // Direct port manipulation speeds taking Slave Select LOW before SPI action. Take the slave-select pin HIGH to avoid conflicts. This is equivalent to the previous line.
}

void MCP::_closeCommLink(void){
  //digitalWrite(CS_MCP,HIGH);                  // Set SS pin high
  PORTB |= 0b00000010;                        // Direct port manipulation speeds taking Slave Select HIGH after SPI action. This is equivalent to the above line.
}

void MCP::openCommLinkForFastRead(uint8_t pin_){
  uint8_t code = ADDR_ENABLE_MCP;
  MCP::pinMode_MCP(pin_,INPUT_MCP);                       // Make sure pin is input CP
  code |= SEQ_DISABLE_MCP;
  MCP::byteWrite(IOCON_MCP, code);
  _openCommLink();
  SPI.transfer(OPCODER_MCP | (MCP::address << 1));  // Send the MCP23S17 opcode, chip address, and read bit
 if(pin_ < 8)
 {
    SPI.transfer(GPIOA_MCP);                      // Send the register we want to read //do an if pin > 7 ... get GPIOB_ else GPIBOa
    MCP::pinForFastRead = (1 << pin_);
 }
 else{
    SPI.transfer(GPIOB_MCP);                      // Send the register we want to read //do an if pin > 7 ... get GPIOB_ else GPIBOa
    MCP::pinForFastRead = (1 << (pin_ - 8));
 }
}

void MCP::closeCommLinkFromFastRead(void){
  SPI.transfer(OPCODEW_MCP | (MCP::address << 1));      //These three lines are to reset the dio chip back to the default setting as in the constructor. It may change ie. openCommLinkForFastRead();
  SPI.transfer(IOCON_MCP);
  SPI.transfer(ADDR_ENABLE_MCP);
  digitalWrite(CS_MCP,HIGH);                  // Set SS pin high
  //PORTB |= 0b00000010;                        // Direct port manipulation speeds taking Slave Select HIGH after SPI action
}


// GENERIC BYTE WRITE - will write a byte to a register, arguments are register address and the value to write
//Don't put "delay()" in the implementation file -CP

void MCP::byteWrite(uint8_t reg_, uint8_t value_) {     // Accept the register and byte
  MCP::_openCommLink();                                       // Sets details necessary to transfer bits to/from the chip. Must call before any set of SPI commumications.
  SPI.transfer(OPCODEW_MCP | (MCP::address << 1));      // Send the MCP23S17 opcode, chip address, and write bit
  SPI.transfer(reg_);                                   // Send the register we want to write
  SPI.transfer(value_);                                 // Send the byte
  MCP::_closeCommLink();                                      // Sets details necessary to transfer bits to/from the chip. Must call after any set of SPI communications.
}

// GENERIC WORD WRITE - will write a word to a register pair, LSB to first register, MSB to next higher value register

void MCP::wordWrite(uint8_t reg_, uint16_t word_) {     // Accept the start register and word
  MCP::_openCommLink();                                       // Sets details necessary to transfer bits to/from the chip. Must call before any set of SPI commumications.
  SPI.transfer(OPCODEW_MCP | (MCP::address << 1));      // Send the MCP23S17 opcode, chip address, and write bit //See sec 1.3.3.1 of MCP datasheet, and Fig. 1-5
  SPI.transfer(reg_);                                   // Send the register we want to write
  SPI.transfer((uint8_t) (word_));                      // Send the low byte (register address pointer will auto-increment after write)
  SPI.transfer((uint8_t) (word_ >> 8));                 // Shift the high byte down to the low byte location and send
  MCP::_closeCommLink();                                      // Sets details necessary to transfer bits to/from the chip. Must call after any set of SPI communications.
}

// MODE SETTING FUNCTIONS - BY PIN AND BY WORD

void MCP::pinMode_MCP(uint8_t pin_, bool modeBool_) {  // Accept the pin # and I/O mode
  if (pin_ > 15) return;               // If the pin value is not valid (1-16) return, do nothing and return
  if (modeBool_ == INPUT_MCP) {                          // Determine the mode before changing the bit state in the mode cache
    MCP::modeCache |= (1 << pin_);               // Since input = "HIGH", OR in a 1 in the appropriate place
  } else {
    MCP::modeCache &= ~(1 << pin_);            // If not, the mode must be output, so and in a 0 in the appropriate place
  }
  MCP::wordWrite(IODIRA_MCP, MCP::modeCache);                // Call the generic word writer with start register and the mode cache
}

void MCP::pinMode_MCP(uint16_t modeWord_) {    // Accept the word
  MCP::wordWrite(IODIRA_MCP, modeWord_);                // Call the the generic word writer with start register and the mode cache
  MCP::modeCache = modeWord_;
}

// THE FOLLOWING WRITE FUNCTIONS ARE NEARLY IDENTICAL TO THE FIRST AND ARE NOT INDIVIDUALLY COMMENTED

// WEAK PULL-UP SETTING FUNCTIONS - BY WORD AND BY PIN

void MCP::pullupMode(uint8_t pin_, bool modeBool_) {
  if (pin_ > 15) return;
  if (modeBool_ == ON_MCP) {
    MCP::pullupCache |= (1 << pin_);
  } else {
    MCP::pullupCache &= ~(1 << pin_);
  }
  MCP::wordWrite(GPPUA_MCP, MCP::pullupCache);
}


void MCP::pullupMode(uint16_t modeWord_) {
  MCP::wordWrite(GPPUA_MCP, modeWord_);
  MCP::pullupCache = modeWord_;
}


// INPUT INVERSION SETTING FUNCTIONS - BY WORD AND BY PIN

void MCP::inputInvert(uint8_t pin_, bool modeBool_) {
  if (pin_ > 15) return;
  if (modeBool_ == ON_MCP) {
    MCP::invertCache |= (1 << pin_);
  } else {
    MCP::invertCache &= ~(1 << pin_);
  }
  MCP::wordWrite(IPOLA_MCP, MCP::invertCache);
}

void MCP::inputInvert(uint16_t modeWord_) {
  MCP::wordWrite(IPOLA_MCP, modeWord_);
  MCP::invertCache = modeWord_;
}


// WRITE FUNCTIONS - BY WORD AND BY PIN

void MCP::digitalWrite(uint8_t pin_, bool valueBool_) {
  if (pin_ > 15) return;
  if (valueBool_)
  {
    MCP::outputCache |= (1 << pin_);
  }
  else
  {
    MCP::outputCache &= ~(1 << pin_);
  }
  //MCP::pinMode_MCP(pin_,OUTPUT_MCP);                        // This cannot be just in the setup.
  MCP::wordWrite(GPIOA_MCP, MCP::outputCache);
}

void MCP::digitalWrite(uint16_t valueWord_) {
  MCP::wordWrite(GPIOA_MCP, valueWord_);
  MCP::outputCache = valueWord_;
}


// READ FUNCTIONS - BY WORD, BYTE AND BY PIN

uint16_t MCP::_digitalRead(void) {                       // This function will read all 16 bits of I/O, and return them as a word in the format 0x(portB)(portA)
  uint16_t value = 0x0000;                              // Initialize a variable to hold the read values to be returned
  MCP::_openCommLink();                                       // Sets details necessary to transfer bits to/from the chip. Must call before any set of SPI commumications.
  SPI.transfer(OPCODER_MCP | (MCP::address << 1));      // Send the MCP23S17 opcode, chip address, and read bit
  SPI.transfer(GPIOA_MCP);                              // Send the register we want to read
  value = SPI.transfer(0x00);                           // Send any byte, the function will return the read value (register address pointer will auto-increment after write)
  value |= (SPI.transfer(0x00) << 8);                   // Read in the "high byte" (portB) and shift it up to the high location and merge with the "low byte"
  //MCP::_closeCommLink();                                      // Sets details necessary to transfer bits to/from the chip. Must call after any set of SPI communications.
  PORTB |= 0b00000010;
  return value;                                         // Return the constructed word, the format is 0x(portB)(portA)
}

bool MCP::digitalReadFast() {            // This function will read all 16 bits of I/O, and return them as a word in the format 0x(portB)(portA)
  return (SPI.transfer(0x00) & MCP::pinForFastRead);                                         // Return the constructed word, the format is 0x(portB)(portA)
}

uint8_t MCP::byteRead(uint8_t reg_) {                   // This function will read a single register, and return it
  uint8_t value = 0x00;                                 // Initialize a variable to hold the read values to be returned
  MCP::_openCommLink();                                       // Sets details necessary to transfer bits to/from the chip. Must call before any set of SPI commumications.
  SPI.transfer(OPCODER_MCP | (MCP::address << 1));      // Send the MCP23S17 opcode, chip address, and read bit
  SPI.transfer(reg_);                                   // Send the register we want to read
  value = SPI.transfer(0x00);                           // Send any byte, the function will return the read value
  MCP::_closeCommLink();                                      // Sets details necessary to transfer bits to/from the chip. Must call after any set of SPI communications.
  return value;                                         // Return the constructed word, the format is 0x(register value)
}

bool MCP::digitalRead(uint8_t pin_) {                   // Return a single bit value, supply the necessary bit (1-16)
    if (pin_ > 15) return false;                        // If the pin value is not valid (1-16) return, do nothing and return
    //MCP::pinMode_MCP(pin_,INPUT_MCP);                       // Make sure pin is input CP
    return  (MCP::_digitalRead() & (1 << pin_)) ? HIGH_MCP : LOW_MCP;  // Call the word reading function, extract HIGH/LOW information from the requested pin. This should return HIGH if there is a HIGH on pin.
}
