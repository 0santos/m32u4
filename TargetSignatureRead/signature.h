#include <avr/pgmspace.h>

// target board reset goes to here
#define RESET PIN_SS
const unsigned long kb = 1024;

// structure to hold signature and other relevant data about each chip
typedef struct {
  byte sig[3];                // chip signature
  const char * desc;          // chip description
  unsigned long flashSize;    // flash size
  byte lfuse;
  byte hfuse;                 // default fuse byte
  byte efuse;
} signatureType;

const signatureType signatures[] PROGMEM =
{
  {{ 0x1E, 0x95, 0xF }, "ATmega328P", 32 * kb, 0x62, 0xd9, 0xff},
  {{ 0x1E, 0x95, 0x87 }, "ATmega32U4", 32 * kb, 0, 0, 0},
  {{ 0x1E, 0x97, 0x6 }, "ATmega1284", 128 * kb, 0x42, 0x99, 0xff},
  {{ 0x1E, 0x91, 0xA }, "ATtiny2313A", 2 * kb, 0, 0, 0}
};
signatureType currentSig;
uint8_t NUMITEMS = sizeof(signatures) / sizeof(signatures[0]);

// programming commands to send via SPI to the chip
enum {
  pgmEnable = 0xAC,

  // writes are preceded by pgmEnable
  chipErase = 0x80,
  writeLockByte = 0xE0,
  writeLowFuseByte = 0xA0,
  writeHighFuseByte = 0xA8,
  writeExtendedFuseByte = 0xA4,

  pollReady = 0xF0,

  pgmAcknowledge = 0x53,

  readSignatureByte = 0x30,
  readCalibrationByte = 0x38,

  readLowFuseByte = 0x50,       readLowFuseByteArg2 = 0x00,
  readExtendedFuseByte = 0x50,  readExtendedFuseByteArg2 = 0x08,
  readHighFuseByte = 0x58,      readHighFuseByteArg2 = 0x08,
  readLockByte = 0x58,          readLockByteArg2 = 0x00,

  readProgramMemory = 0x20,
  writeProgramMemory = 0x4C,
  loadExtendedAddressByte = 0x4D,
  loadProgramMemory = 0x40,
  readEEPROMbyte = 0xA0,
  writeEEPROMbyte = 0xC0
};
