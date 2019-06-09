/*
  Arduino Micro target board signature read.
    base on Nick Gammon sketch

  Connect target processor like this:

  ATmega32U4(Leonardo, Micro, etc..)
    D14(MISO)  --> MISO as per datasheet
    D15(SCK)   --> SCK as per datasheet
    D16(MOSI)  --> MOSI as per datasheet
    D17(SS)    --> goes to /RESET on target

  Target processor will need +5V and Gnd connected.

*/
#if !defined __AVR_ATmega32U4__
#error Unsupported board selection
#endif

#include "spi.h"
#include "signature.h"
#include <util/delay.h>
#include <avr/interrupt.h>

void setup() {
  Serial.begin(115200);
  // if Leonardo or similar wait for serial monitor connection
  while (!Serial) {
    ;
  }

  Serial.println(F("AVR SignatureRead Ready"));
}

void loop() {
  Serial.println(F("Type [R] Read Board Information ..."));

  while (toupper(Serial.read()) != 'R') {}

  readALlFromChip();
}

void readALlFromChip() {
  Serial.println(F("--------- Starting ---------"));
  Serial.println();
  if (!startProgramming()) {
    Serial.println(F("NO BOARD DETECTED!!"));
    return;
  }

  byte sig[3];
  Serial.print(F("Signature = "));

  readSignature(sig);
  for (byte i = 0; i < 3; i++) {
    Serial.print("0x");
    Serial.print(sig[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  for (uint8_t j = 0; j < NUMITEMS; j++) {
    memcpy_P(&currentSig, &signatures[j], sizeof(currentSig));

    if (memcmp(sig, currentSig.sig, sizeof(sig)) == 0) {
      Serial.print(F("Processor = "));
      Serial.println(currentSig.desc);
      Serial.print(F("Flash memory size = "));
      Serial.print(currentSig.flashSize, DEC);
      Serial.println(F(" bytes."));
    }
  }

  getFuseBytes();

  stopProgramming();
  Serial.println();
  Serial.println(F("--------- Complete ---------"));
  Serial.println("\n");
}

// execute one programming instruction... b1 is command, b2, b3, b4 are arguments
// processor may return a result on the 4th transfer, this is returned
uint8_t spi_transaction(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
  spi_transmit(b1);
  spi_transmit(b2);
  spi_transmit(b3);
  return spi_transmit(b4);
}

// put chip into programming mode
// returns true if managed to enter programming mode
bool startProgramming() {
  Serial.print(F("Attempting to enter ICSP programming mode ..."));

  byte confirm;

  spi_begin(SPI_CLOCK_DIV64, SPI_MODE0);

  uint8_t timeout = 0;
  // we are in sync if we get back pgmAcknowledge on the third byte
  do {
    // regrouping pause
    _delay_ms(100);

    // ensure PIN_SCK low
    cli();
    //noInterrupts();
    PORTB &= ~_BV(PIN_SCK);
    //digitalWrite(PIN_SCK, LOW);

    // then pulse reset, see page 309 of datasheet
    PORTB |= _BV(RESET);
    _delay_us(10);  // pulse for at least 2 clock cycles
    PORTB &= ~_BV(RESET);

    sei();
    //interrupts();

    _delay_ms(25);  // wait at least 20 mS
    cli();
    //noInterrupts();

    spi_transmit(pgmEnable);
    spi_transmit(pgmAcknowledge);
    confirm = spi_transmit(0x0);
    spi_transmit(0x0);

    sei();
    //interrupts();

    if (confirm != pgmAcknowledge) {
      Serial.print(".");
      if (timeout++ >= 2) {
        Serial.println();
        Serial.println(F("Failed to enter programming mode. Double-check wiring!"));
        return false;
      }
    }

  } while (confirm != pgmAcknowledge);

  Serial.println();
  Serial.println(F("Entered programming mode [OK]."));
  return true;
}

void stopProgramming() {
  spi_end();
  // We're about to take the target out of reset so configure SPI pins as input
  DDRB &= ~(_BV(PIN_MOSI) | _BV(PIN_SCK));
  //pinMode(PIN_MOSI, INPUT);
  //pinMode(PIN_SCK, INPUT);

  PORTB &= ~_BV(RESET);
  //digitalWrite(RESET, LOW);
  DDRB &= ~_BV(RESET);
  //pinMode(RESET, INPUT);

  Serial.println(F("Programming mode off."));
}

void readSignature(byte sig[3]) {
  for (byte i = 0; i < 3; i++) {
    sig[i] = spi_transaction(readSignatureByte, 0x0, i, 0x0);
  }
  spi_transaction(loadExtendedAddressByte, 0x0, 0x0, 0x0);
}

void getFuseBytes() {
  byte fuses[5];
  fuses[0] = spi_transaction(readLowFuseByte, readLowFuseByteArg2, 0x0, 0x0);
  fuses[1] = spi_transaction(readHighFuseByte, readHighFuseByteArg2, 0x0, 0x0);
  fuses[2] = spi_transaction(readExtendedFuseByte, readExtendedFuseByteArg2, 0x0, 0x0);
  fuses[3] = spi_transaction(readLockByte, readLockByteArg2, 0x0, 0x0);
  fuses[4] = spi_transaction(readCalibrationByte, 0x0, 0x0, 0x0);

  Serial.print(F("LFuse = 0x"));
  Serial.println(fuses[0], HEX);
  Serial.print(F("HFuse = 0x"));
  Serial.println(fuses[1], HEX);
  Serial.print(F("EFuse = 0x"));
  Serial.println(fuses[2], HEX);
  Serial.print(F("Lock byte = 0x"));
  Serial.println(fuses[3], HEX);
  Serial.print(F("Clock calibration = "));
  Serial.println(fuses[4], HEX);
}

// Simply polls the chip until it is not busy any more - for erasing and programming
/*void busyWait() {
  byte busybit;
  do {
    busybit = spi_transaction(0xF0, 0x0, 0x0, 0x0); // program
    Serial.print(busybit, HEX);
  } while (busybit & 0x01);
  }*/
