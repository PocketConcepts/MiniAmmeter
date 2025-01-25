//

// Moved to bitbang library because I connected the wrong pins on the attiny45 :(

//

#include <I2CTinyBB.h>  // Use I2CTinyBB instead of TinyWireM
#include <avr/pgmspace.h>  // Include the pgmspace.h library to use PROGMEM

#define OLED_ADDR 0x3C // I2C address for the OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

void sendCommand(uint8_t command) {
  uint8_t cmd[] = {0x00, command}; // Command mode and command
  I2CWrite(OLED_ADDR, cmd, sizeof(cmd)); // Write the command
}

void sendData(uint8_t data) {
  uint8_t dat[] = {0x40, data}; // Data mode and data
  I2CWrite(OLED_ADDR, dat, sizeof(dat)); // Write the data
}

void setup() {
  I2CInit(3, 4, 1); // Initialize I2C with custom SDA and SCL pins and optional delay count

  // OLED initialization sequence
  sendCommand(0xAE); // Display off
  sendCommand(0xD5); sendCommand(0x80); // Set display clock divide ratio
  sendCommand(0xA8); sendCommand(0x1F); // Set multiplex
  sendCommand(0xD3); sendCommand(0x00); // Set display offset
  sendCommand(0x40); // Set display start line
  sendCommand(0x8D); sendCommand(0x14); // Charge pump
  sendCommand(0xA1); // Set segment re-map
  sendCommand(0xC8); // COM output scan direction
  sendCommand(0xDA); sendCommand(0x02); // Set COM pins hardware config
  sendCommand(0x81); sendCommand(0xCF); // Set contrast control
  sendCommand(0xD9); sendCommand(0xF1); // Set pre-charge period
  sendCommand(0xDB); sendCommand(0x40); // Set VCOMH deselect level
  sendCommand(0xA4); // Entire display on
  sendCommand(0xA6); // Set normal display
  sendCommand(0xAF); // Display ON

  clearScreen();
}

void loop() {
  // Cycle through the letters 'A', 'B', 'C' at position (8, 1) every second
  drawChar(8, 0, 0);
  delay(500);  
  drawChar(16, 1, 1);
  delay(500);  
  drawChar(24, 2, 2);
  delay(500);  
  clearScreen();
  delay(500);

  drawLargeChar(0);
  delay(500);
  clearScreen();

}

void clearScreen() {
  for (uint8_t page = 0; page < 4; page++) {  // 4 pages for 32 pixels height
    sendCommand(0xB0 + page);  // Set the page address (0xB0 is the first page)
    sendCommand(0x00);         // Set lower column address
    sendCommand(0x10);         // Set higher column address
    for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
      sendData(0x00);  // Send blank data to each column
    }
  }
}
void drawChar(uint8_t x, uint8_t y, uint8_t c) {
  static const uint8_t font[] PROGMEM = { // Use PROGMEM to store the font data in flash memory instead of SRAM
    0x7C, 0x82, 0x82, 0x82, 0x7C, 0x00,   // '0'
    0xFE, 0x92, 0x92, 0x92, 0x6C, 0x00,   // 'B'
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x00,   // 'C'
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00,   // '.'

};

  // Set the cursor position using commands
  sendCommand(0xB0 + y);        // Set the page address (y is the page)
  sendCommand(0x00 + (x & 00001111)); // bitwise AND the last 4 bits to set the lower column start address (columns 0-15)
  sendCommand(0x10 + (x >> 4));  // Set higher column address (shift bits right 4) (columns 16-127)

  // Draw the character
  uint8_t index = (c * 6);  // Calculate the index based on 'A'
  for (uint8_t i = 0; i < 6; i++) {
    sendData(pgm_read_byte(&font[index + i])); // Use pgm_read_byte to read from PROGMEM
  }

}
void drawLargeChar(uint8_t x) {
  static const uint8_t font[] PROGMEM = { // Use PROGMEM to store the font data in flash memory instead of SRAM
    0xFF, 0x03, 0x05, 0x09, 0x11, 0x21, 0x41, 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
  };


 // Iterate over 4 pages
  for (uint8_t page = 0; page < 4; page++) {
    sendCommand(0xB0 + page);               // Set page address
    sendCommand(0x00 + (x & 00001111));         // Set lower column start address
    sendCommand(0x10 + (x >> 4));           // Set higher column start address

    //Draw the character across 4 pages
    for (uint8_t i = 0; i < 24; i++) {
      sendData(pgm_read_byte(&font[page * 24 + i])); // Use pgm_read_byte to read from PROGMEM

    }
  }
}
