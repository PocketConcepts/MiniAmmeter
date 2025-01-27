
#include <I2CTinyBB.h>  // Use I2CTinyBB instead of TinyWireM
#include <avr/pgmspace.h>  // Include the pgmspace.h library to use PROGMEM
#include <avr/io.h>

#define OLED_ADDR 0x3C // I2C address for the OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//Shunt resistors
#define R_1 0.1
#define R_10 1

// Diff amp resistors
#define R_2 100000
#define R_3 10000
#define R_4 10000
#define R_5 100000

//Vref buffer resistors
#define R_13 20000
#define R_15 20000

//ADC Pinmap
#define VREF PB0 // ( EVAL Mod required if using analogRead(EXTERNAL) )
#define VOUT_PIN 1 // ( PCB Mod required ) 
#define VCC_PIN 0 // ( PCB Mod required ) 

// Kerning offset
uint8_t  kerning = 4;

// Array to store digit positions
uint8_t  digit_position[7];

///////////////////////////////////////////////////////////////////////////////////////////////

//Calculate gain based on resistor values
float calculateGain() {
  return (R_2 / R_3);
}

//Calculate Vref based on resistor values
float calculateResistiveDividerRatio() {
  return ((R_13) / (R_15 + R_13));
}

uint16_t readADC(uint8_t pin, uint8_t windowSize) {
    uint16_t sum = 0;
    uint16_t samples[windowSize];  // Array to store samples in SRAM
    for (uint8_t i = 0; i < windowSize; i++) {
        samples[i] = analogRead(pin);  // Read ADC value
        sum += samples[i];
    }
    return sum / windowSize;  // Return the average
}

// Calculate ILOAD based on transfer function
float calculateILOAD(uint8_t averagingWindow) {

  // Read VOUT using the ADC
  uint16_t VOUT_ADC = readADC(VOUT_PIN, averagingWindow); 

  // Read VCC using the ADC (only read once for consistency)
  uint16_t VCC_ADC = readADC(VCC_PIN, averagingWindow); 

  // Calculate VREF (~2.5V) based on VCC and the resistive divider ratio
  uint16_t VREF_ADC = VCC_ADC / calculateResistiveDividerRatio();

  // Calculate gain
  float GAIN = calculateGain();

  // Apply transfer function
  //float ILOAD = ((VOUT_ADC - VREF_ADC) / GAIN) / R_1;
  float ILOAD = VOUT_ADC - 511.0f;
  return ILOAD;

}

int getDigitAtPosition(int value, int position) {
    int digits[10]; // Assuming value won't exceed 10 digits
    int index = 0;

    // Extract digits into the array
    while (value > 0) {
        digits[index++] = value % 10;  // Extract last digit
        value /= 10;                  // Remove last digit
    }

    // Check for invalid position
    if (position < 0 || position >= index) {
        return -1; // Return -1 to indicate an invalid position
    }

    // Return the digit at the position (adjusted for reverse order)
    return digits[index - position - 1];
}

// Function to calculate the positions
void calculate_digit_position() {
    digit_position[0] = -8;
    digit_position[1] = 8;
    // Calculate positions for digits 2 to 5
    for (int i = 2; i < 7; i++) {
        digit_position[i] = digit_position[i - 1] + 24 - kerning;
    }
}

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

  pinMode(VREF, INPUT);   // PB2 as input
  pinMode(VOUT_PIN, INPUT);  // Analogue ref as input
  pinMode(VCC_PIN, INPUT);  // Analogue ref as input

  //Take VCC as VREF
  analogReference(DEFAULT);

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

  // Call function to calculate digit positions
  calculate_digit_position();

  float ILOAD = calculateILOAD(1); 
  
  drawRightAlignedNumber(ILOAD);

  drawLargeChar(digit_position[5], 'm');
  drawLargeChar(108, 'A');

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

void drawNegative(float load, int position) {
  if (load < 0) 
    drawLargeChar(position, '-');
    
}

// Function to draw a right-aligned number
void drawRightAlignedNumber(int number) {
    int numDigits = 0;
    int temp = abs(number); // Use absolute value to count digits

    // Count how many digits the number has
    do {
        temp /= 10;
        numDigits++;
    } while (temp > 0);

    // Right-align the digits by starting from the rightmost position
    for (uint8_t i = 0; i < numDigits; i++) {
        int digit = getDigitAtPosition(abs(number), i);  // Extract the digit at position i
        // Adjust the position for right alignment
        drawLargeChar(digit_position[5 - numDigits + i], digit);
    }

    // Draw the negative sign if the number is negative
    if (number < 0) {
        drawNegative(number, digit_position[0]); 
    }
}

void drawLargeChar(uint8_t x, uint8_t c) {
  static const uint8_t font[] PROGMEM = { // Use PROGMEM to store the font data in flash memory instead of SRAM

    // 0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x80, 0x80, 0xE0, 0xF0, 0xF0, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xCC, 0x0E, 0x0F, 0x07, 0x07, 0x01, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    // 1
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   
    // 2
    0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x83, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xE3, 0xFF, 0xFF, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFC, 0xFE, 0xFF, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,

    // 3
    0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xE3, 0xFF, 0xFF, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0xC7, 0xFF, 0xFF, 0x7C, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    // 4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xF8, 0x7C, 0x7C, 0x1F, 0x1F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0xFF, 0xFF, 0xFF, 0xFF, 0x1C, 0x1C, 0x1C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // 5
    0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x3F, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // 6
    0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x83, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x83, 0x87, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xC3, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xC7, 0xFF, 0xFF, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // 7
    0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xF8, 0xFF, 0x3F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xE3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE3, 0xFF, 0xFF, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xC7, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x8F, 0xFF, 0xFF, 0xF8, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // 9
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xE3, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x83, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE1, 0xC3, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // m
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0xFF, 0xFF, 0x03, 0x07, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // A
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xFC, 0xFC, 0x1F, 0x07, 0x07, 0x03, 0x03, 0x03, 0x07, 0x07, 0x1F, 0xFF, 0xFC, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,

    // -    
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // .
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  };


  // Define the mapping from characters to font array indices
  uint32_t index;
  
  switch (c) {
    case 0: index = 0; break;
    case 1: index = 1; break;
    case 2: index = 2; break;
    case 3: index = 3; break;
    case 4: index = 4; break;
    case 5: index = 5; break;
    case 6: index = 6; break;
    case 7: index = 7; break;
    case 8: index = 8; break;
    case 9: index = 9; break;
    case 'm': index = 10; break;
    case 'A': index = 11; break;
    case '-': index = 12; break;
    case '.': index = 13; break;
    default: index = 0; break;  // Default to '0' if character is not recognized

  }

  uint32_t offset = index * 24 * 4; // Calculate starting index position for character c

  // Iterate over 4 pages (rows of 8 pixels each)
  for (uint8_t page = 0; page < 4; page++) {
    sendCommand(0xB0 + page);               // Set page address
    sendCommand(0x00 + (x & 0x0F));         // Set lower column start address
    sendCommand(0x10 + (x >> 4));           // Set higher column start address

    // Draw the character data for this page
    for (uint8_t i = 0; i < 24; i++) {
      uint8_t data = pgm_read_byte(&font[offset + page * 24 + i]);
      sendData(data);
    }
  }
}
 
