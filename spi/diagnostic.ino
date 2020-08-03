#include <avr/io.h>
#include <util/delay.h>

uint8_t buff[20] = {0x00, 0x5F, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x36, 0x49, 0x56, 0x20, 0x50, 0x22, 0x14, 0x7F, 0x14, 0x22, 0x40, 0x30, 0x10};

// The ATmega32U4 SPI pins
#if defined(__AVR_ATmega32U4__)
//#define SS_DDR    DDRB
//#define SS_PORT   PORTB
//#define SS_BIT    6    // PB6, pin 30, Digital 10, we can use another pin as SS, Digital 10 in this case

#define SPI_MISO  PB3    // PB3, pin 11, Digital 14
#define SPI_SCK   PB1    // PB1, pin 9, Digital 15
#define SPI_MOSI  PB2    // PB2, pin 10, Digital 16
#define SPI_SS    PB0    // PB0, pin 8, Digital 17, the SS pin it's used for the Rx LED
#else
#error MCU is not supported
#endif

#define SS_HIGH {PORTB |= (1 << SPI_SS);}
#define SS_LOW  {PORTB &= ~(1 << SPI_SS);}

void spi_init_master() {
  // SPI comunication test, MOSI and MISO are connected together, set a jumper wire between these pin

  // Set MOSI, SCLK, and SS as output
  DDRB |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS);

  // Set MISO as input
  DDRB &= ~(1 << SPI_MISO);

  // Set SS high, release chip
  SS_HIGH;

  // Enable SPI, Master mode, prescaler = /16:
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint8_t spi_transfer(volatile uint8_t data) {
  uint8_t r = 0;
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1 << SPIF)));  // Wait the end of the transmission
  r = SPDR;
  return r;                       // return the received byte
}

uint8_t spi_read_write(uint8_t value) {
  uint8_t d = 0;
  SS_LOW;
  d = spi_transfer(value);  // get data byte
  SS_HIGH;                  // release chip, signal end transfer
  return d;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  _delay_ms(100);

  spi_init_master();
  _delay_ms(10);

  Serial.println("************SPI communication begin************");
  Serial.println();
}

void loop() {
  for (uint8_t i = 0; i < sizeof(buff); i++) {
    Serial.print(spi_read_write(buff[i]), HEX);
    Serial.print(" ");
  }
  Serial.println();

  _delay_ms(1000);
}
