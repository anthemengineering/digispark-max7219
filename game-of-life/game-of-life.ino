/*
 * Digispark MAX7219 - Game of Life
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

#define ROWS 8
#define COLS 8
#define FERTILITY 30

uint8_t today[8], tomorrow[8], yesterday[8];

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

void reveal() {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    max7219_writec(i + 1, today[i]);
  }
}

uint8_t alive_neighbors(uint8_t row, uint8_t col) {
  uint8_t alive = 0;
  uint8_t col_mask = 0x80 >> col;

  // above
  if (row > 0) {
    // left
    if (col > 0) {
      alive += !!(today[row - 1] & (col_mask << 1));
    }

    // center
    alive += !!(today[row - 1] & (col_mask));
    
    // right
    if (col < (COLS - 1)) {
      alive += !!(today[row - 1] & (col_mask >> 1));
    }
  }

  // current row
  // left
  if (col > 0) {
    alive += !!(today[row] & (col_mask << 1));
  }

  // center
  alive += !!(today[row] & (col_mask));
  
  // right
  if (col < (COLS - 1)) {
    alive += !!(today[row] & (col_mask >> 1));
  }

  // below
  if (row < (ROWS - 1)) {
    // left
    if (col > 0) {
      alive += !!(today[row + 1] & (col_mask << 1));
    }

    // center
    alive += !!(today[row + 1] & (col_mask));
    
    // right
    if (col < (COLS - 1)) {
      alive += !!(today[row + 1] & (col_mask >> 1));
    }
  }

  return alive;
}

uint8_t determine_fate(uint8_t row, uint8_t col) {
  uint8_t self = !!(today[row] & (0x80 >> col));
  uint8_t friends = alive_neighbors(row, col);

  if (self) {
    if (friends < 2 || friends > 3) {
      return 0;
    } else {
      return 1;
    }    
  } else {
    if (friends == 3) {
      return 1;
    } else {
      return 0;
    }
  }
}

void hand_of_god() {
  uint8_t row, col, value, mask;

  for (row = 0; row < ROWS; row++) {
    value = 0;
    
    for (col = 0, mask = 0x80; col < COLS; col++, mask >>= 1) {
      if (determine_fate(row, col)) {
        value |= mask;
      }
    }

    tomorrow[row] = value;
  }

  for (row = 0; row < ROWS; row++) {
    yesterday[row] = today[row];
    today[row] = tomorrow[row];
  }
}

void let_there_be_light() {
  uint8_t row, col, value, mask;

  for (row = 0; row < ROWS; row++) {
    value = 0;
    
    for (col = 0, mask = 0x80; col < COLS; col++, mask >>= 1) {
      if (random(100) < FERTILITY) {
        value |= mask;
      }
    }

    today[row] = value;
  }
}

uint8_t a_new_day() {
  uint8_t row;

  for (row = 0; row < ROWS; row++) {
    if (today[row] != yesterday[row]) {
      return 1;
    }
  }

  return 0;
}

void setup() {
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

  randomSeed(micros());

  let_there_be_light();
  reveal();
}

void loop(){
  hand_of_god();

  if (!a_new_day()) {
    let_there_be_light();
  }
  
  reveal();

  delay(1000);
}
