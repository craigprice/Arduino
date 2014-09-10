#define SPIRAM_LIB_VERSION "0.1.4"
//    FILE: SpiRAM.h
//  AUTHOR: K. Tyler
// VERSION: 0.1.4
// PURPOSE: Arduino library for the 23A1024/23LC1024 SRAM memory chip
//     URL:
//
// HISTORY:
// see SpiRAM.cpp file

#ifndef SpiRAM_h
#define SpiRAM_h

// support for old IDE's
#if ARDUINO < 100
  #include <WProgram.h>
#else
  #include <Arduino.h>
#endif

#include <SPI.h>

// SRAM Instructions
const static uint8_t RDMR_SRAM  = 0x05;//Read the control register for mode
const static uint8_t WRMR_SRAM  = 0x01;//Write to the control register for mode
const static uint8_t WRITE_SRAM = 0b00000010;
const static uint8_t READ_SRAM  = 0b00000011;

// SRAM modes
const static uint8_t BYTE_MODE_SRAM       = 0b00000000;
const static uint8_t PAGE_MODE_SRAM       = 0b10000000;
const static uint8_t SEQUENTIAL_MODE_SRAM = 0b01000000;//default is sequential.

const static int     CS_SRAM              = 8;          // SPI bus slave select output to pin


class SpiRAM {
  public:
    // Initialize and specify the SS pin
    SpiRAM          (void);

    // Change SS pin (used in multi SRAM chip)
    //byte changeSS   (byte SS_SRAM);

    // Read a single byte from address
    byte readByte   (uint32_t address);

    // Write a single byte to address
    void writeByte  (uint32_t address, byte data_byte);

    // Read several bytes starting at address and of length into a uint8_t buffer
    void readBuffer (uint32_t address, uint8_t *buffer, uint32_t length);

    // Write several bytes starting at address and of length from a uint8_t buffer
    void writeBuffer(uint32_t address, uint8_t *buffer, uint32_t length);

    // Write several bytes of value, starting at address and of length
    void fillBytes  (uint32_t address, byte value, uint32_t length);

    void setAddressMode(uint32_t address, byte mode);

  private:
};

#endif
// END OF FILE
