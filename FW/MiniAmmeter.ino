//

// Moved to bitbang library because I connected the wrong pins on the attiny45 :(

//

#include <I2CTinyBB.h>  // Use I2CTinyBB instead of TinyWireM

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
  drawChar(8, 1, 'A');
  delay(500);  // Delay to make it visible for 1 second
  drawChar(8, 1, 'B');
  delay(500);  // Delay to make it visible for 1 second
  drawChar(8, 1, 'C');
  delay(500);  // Delay to make it visible for 1 second
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

void drawChar(uint8_t x, uint8_t y, char c) {
  static const uint8_t font[] = {
    // 6x8 font definition for 'A', 'B', and 'C' (add more characters as needed)
    0xFC, 0x22, 0x22, 0x22, 0xFC, 0x00,  // 'A'
    0xFE, 0x92, 0x92, 0x92, 0x6C, 0x00,  // 'B'
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x00,  // 'C'
  };

  uint8_t index = (c - 'A') * 6;  // Calculate the index based on 'A'
  
  for (uint8_t i = 0; i < 6; i++) {
    sendData(font[index + i]); // Send each byte of the character data
  }
}
