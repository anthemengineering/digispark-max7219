/*
 * Digispark MAX7219
 *
 * Uses a Digispark (an ATTiny85) to control an LED array via an MAX7219 controller using
 * a software-controlled SPI interface. This sketch assumes the following pin connections
 *
 * Digispark <--> MAX7219
 * 5V.............VCC
 * GND............GND
 * P0.............DIN/MOSI
 * P1.............CS/SS
 * P2.............CLK/SCK
 *
 * See this tutorial for details on the Serial Peripheral Interface (SPI)
 * https://learn.sparkfun.com/tutorials/serial-peripheral-interface-spi
 */

// MOSI is the "master-out/slave-in" line and also known as DATA, DO, DOUT, and LOAD
#define MOSI_HIGH() PORTB |= (1<<PB0)
#define MOSI_LOW()  PORTB &= ~(1<<PB0)

// SS is the "slave select" line and also known as CS
#define SS_HIGH()   PORTB |= (1<<PB1)
#define SS_LOW()    PORTB &= ~(1<<PB1)

// SCK is the "serial clock" line and also known as SCLK, and CLK
#define SCK_HIGH()  PORTB |= (1<<PB2)
#define SCK_LOW()   PORTB &= ~(1<<PB2)

// This sets PB0, PB1, and PB2 to output mode
#define INIT_PORT() DDRB |= (1<<PB0) | (1<<PB1) | (1<<PB2)

#define NUM_FRAMES 6

uint8_t frames[NUM_FRAMES][8] = {
  {
    // A
    0b00000000,
    0b00111100,
    0b01100110,
    0b01100110,
    0b01111110,
    0b01100110,
    0b01100110,
    0b01100110
  }, {
    // N
    0b00000000,
    0b01100011,
    0b01110011,
    0b01111011,
    0b01101111,
    0b01100111,
    0b01100011,
    0b01100011
  }, {
    // T
    0b00000000,
    0b01111110,
    0b01111110,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000
  }, {
    // H
    0b00000000,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01111110,
    0b01100110,
    0b01100110,
    0b01100110
  }, {
    // E
    0b00000000,
    0b01111110,
    0b01111110,
    0b01100000,
    0b01111100,
    0b01100000,
    0b01111110,
    0b01111110
  }, {
    // M
    0b00000000,
    0b01100011,
    0b01110111,
    0b01111111,
    0b01101011,
    0b01100011,
    0b01100011,
    0b01100011
  }
};

uint8_t frame_num;
uint8_t display[8];

void spi_send(uint8_t data) {
  uint8_t i;

  for (i = 0; i < 8; i++, data <<= 1) {
    SCK_LOW();

    if (data & 0x80) {
      MOSI_HIGH();
    } else {
      MOSI_LOW();
    }

    SCK_HIGH();
  }
}

void max7219_writec(uint8_t high_byte, uint8_t low_byte) {
  SS_LOW();
  spi_send(high_byte);
  spi_send(low_byte);
  SS_HIGH();
}

void max7219_clear() {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    max7219_writec(i + 1, 0);
  }
}

void update_display() {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    max7219_writec(i + 1, display[i]);
  }
}

void image(uint8_t im[8]) {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    display[i] = im[i];
  }
}

void setup() {
  frame_num = 0;

  INIT_PORT();

  // See the MAX7219 datasheet for documentation for these values
  // https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf

  // Decode mode: none
  max7219_writec(0x09, 0);

  // Intensity: 3 (0-15)
  max7219_writec(0x0A, 1);

  // Scan limit: All "digits" (rows) on
  max7219_writec(0x0B, 7);

  // Shutdown register: Display on
  max7219_writec(0x0C, 1);

  // Display test: off
  max7219_writec(0x0F, 0);

  max7219_clear();
}

void loop(){
  // Load current frame into display
  image(frames[frame_num]);

  // Display current frame
  update_display();

  delay(750);

  // Advance to next frame
  frame_num = (frame_num + 1) % NUM_FRAMES;
}
