/*
  MCP23S17.h  Version 0.1
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

#ifndef MCP23S17_h
#define MCP23S17_h

// REGISTERS ARE DEFINED HERE SO THAT THEY MAY BE USED IN THE MAIN PROGRAM

const static uint8_t    IODIRA_MCP    = 0x00;      // MCP23x17 I/O Direction Register
const static uint8_t    IODIRB_MCP    = 0x01;      // 1 = Input (default), 0 = Output

const static uint8_t    IPOLA_MCP     = 0x02;      // MCP23x17 Input Polarity Register
const static uint8_t    IPOLB_MCP     = 0x03;      // 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)

const static uint8_t    GPINTENA_MCP  = 0x04;      // MCP23x17 Interrupt on Change Pin Assignements
const static uint8_t    GPINTENB_MCP  = 0x05;      // 0 = No Interrupt on Change (default), 1 = Interrupt on Change

const static uint8_t    DEFVALA_MCP   = 0x06;      // MCP23x17 Default Compare Register for Interrupt on Change
const static uint8_t    DEFVALB_MCP   = 0x07;      // Opposite of what is here will trigger an interrupt (default = 0)

const static uint8_t    INTCONA_MCP   = 0x08;      // MCP23x17 Interrupt on Change Control Register
const static uint8_t    INTCONB_MCP   = 0x09;      // 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)

const static uint8_t    IOCON_MCP     = 0x0A;      // MCP23x17 Configuration Register
//                                     (0x0B)      //     Also Configuration Register

const static uint8_t    GPPUA_MCP     = 0x0C;      // MCP23x17 Weak Pull-Up Resistor Register
const static uint8_t    GPPUB_MCP     = 0x0D;      // INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up

const static uint8_t    INTFA_MCP     = 0x0E;      // MCP23x17 Interrupt Flag Register
const static uint8_t    INTFB_MCP     = 0x0F;      // READ ONLY: 1 = This Pin Triggered the Interrupt

const static uint8_t    INTCAPA_MCP   = 0x10;      // MCP23x17 Interrupt Captured Value for Port Register
const static uint8_t    INTCAPB_MCP   = 0x11;      // READ ONLY: State of the Pin at the Time the Interrupt Occurred

const static uint8_t    GPIOA_MCP     = 0x12;      // MCP23x17 GPIO Port Register
const static uint8_t    GPIOB_MCP     = 0x13;      // Value on the Port - Writing Sets Bits in the Output Latch

const static uint8_t    OLATA_MCP     = 0x14;      // MCP23x17 Output Latch Register
const static uint8_t    OLATB_MCP     = 0x15;      // 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value!

// Defines to keep logical information symbolic go here

const static bool       HIGH_MCP         = 1;
const static bool       ON_MCP           = 1;
const static bool       INPUT_MCP        = 1;

const static bool       LOW_MCP          = 0;
const static bool       OFF_MCP          = 0;
const static bool       OUTPUT_MCP       = 0;


// Here we have things for the SPI bus configuration

const static uint16_t   CLOCK_DIVIDER_MCP   = SPI_CLOCK_DIV2;           // SPI bus speed to be 1/2 of the processor clock speed - 8MHz on most Arduinos
const static int        CS_MCP              = 9;          // SPI bus slave select output to pin 9 - READ ARDUINO SPI DOCS BEFORE CHANGING!!! THE PORTB manipulations implement the chip-select line!!  NDG

// Control byte and configuration register information - Control Byte: "0100 A2 A1 A0 R/W" -- W=0

const static uint8_t    OPCODEW_MCP         = 0b01000000;  // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
const static uint8_t    OPCODER_MCP         = 0b01000001;  // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3
const static uint8_t    ADDR_ENABLE_MCP     = 0b00001000;  // Configuration register for MCP23S17, enabling hardware addressing
const static uint8_t    SEQ_DISABLE_MCP     = 0b00100000;  // Configuration register for MCP23S17, disabling incrementing the Address Pointer (default is to automatically increment after each transfer)

class MCP {
  public:
    MCP(void);                            // Constructor to instantiate a discrete IC as an object, argument is the address (0-7) of the MCP23S17
    void openCommLinkForFastRead (uint8_t pin);
    void closeCommLinkFromFastRead(void);
    void wordWrite(uint8_t, uint16_t);   // Typically only used internally, but allows the user to write any register pair if needed, so it's public
    void byteWrite(uint8_t, uint8_t);        // Typically only used internally, but allows the user to write any register if needed, so it's public
    void pinMode_MCP(uint8_t, bool);          // Sets the mode (input or output) of a single I/O pin
    void pinMode_MCP(uint16_t);              // Sets the mode (input or output) of all I/O pins at once
    void pullupMode(uint8_t, bool);       // Selects internal 100k input pull-up of a single I/O pin
    void pullupMode(uint16_t);           // Selects internal 100k input pull-up of all I/O pins at once
    void inputInvert(uint8_t, bool);      // Selects input state inversion of a single I/O pin (writing 1 turns on inversion)
    void inputInvert(uint16_t);          // Selects input state inversion of all I/O pins at once (writing a 1 turns on inversion)
    void digitalWrite(uint8_t, bool);     // Sets an individual output pin HIGH or LOW
    void digitalWrite(uint16_t);         // Sets all output pins at once. If some pins are configured as input, those bits will be ignored on write
    bool digitalRead(uint8_t);            // Reads an individual input pin
    bool    digitalReadFast(void);
    uint8_t byteRead(uint8_t);               // Reads an individual register and returns the byte. Argument is the register address
        uint16_t    _digitalRead(void);          // Reads all input  pins at once. Be sure it ignore the value of pins configured as output!


    void        _openCommLink(void);
    void        _closeCommLink(void);
    uint16_t    pullupCache;               // Caches the internal pull-up configuration of input pins (values persist across mode changes)
    uint16_t    invertCache;               // Caches the input pin inversion selection (values persist across mode changes)
    uint8_t     pinForFastRead;
	uint16_t    outputCache;               // Caches the output pin state of pins //CP
	uint16_t    modeCache;                 // Caches the mode (input/output) configuration of I/O pins //CP
    uint8_t     address;                   // Address of the MCP23S17 in use
  private:

 };

#endif //MCP23S17
