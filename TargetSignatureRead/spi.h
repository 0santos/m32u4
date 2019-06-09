#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>

#define PIN_MOSI PB2  // MOSI Serial Data In
#define PIN_MISO PB3  // MISO Serial Data Out
#define PIN_SCK  PB1  // Serial Clock
#define PIN_SS   PB0  // Chip Select

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE_MASK 0x0C     // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03    // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

void spi_begin(uint8_t rate, uint8_t mode);
uint8_t spi_transmit(uint8_t data);
void spi_end();

// SPI initialize
void spi_begin(uint8_t rate, uint8_t mode)
{
  // Set MOSI, SCK, SS outputs
  DDRB |= _BV(PIN_MOSI) | _BV(PIN_SCK) | _BV(PIN_SS);

  // Set MISO inputs
  DDRB &= ~_BV(PIN_MISO);

  // Set SS to high so a connected chip will be "deselected" by default
  PORTB |= _BV(PIN_SS);

  // Set Mode
  SPCR = (SPCR & ~SPI_MODE_MASK) | mode;

  // Enable SPI, Master
  SPCR = _BV(SPE) | _BV(MSTR);

  // Set clock rate
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}

uint8_t spi_transmit(uint8_t data)
{
  // Start transmission
  SPDR = data;

  asm volatile("nop");
  // Wait for transmission complete
  while (!(SPSR & _BV(SPIF)));

  // Return data register
  return SPDR;
}

// Disable the SPI bus
void spi_end()
{
  SPCR &= ~_BV(SPE);
}

#endif /* SPI_H_ */
