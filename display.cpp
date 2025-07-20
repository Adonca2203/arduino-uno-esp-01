#include "display.h"
#include "fonts.h"

const int DISPLAY_ADDR = 0x3C;
const uint8_t COMMAND = 0x00;
const uint8_t DATA = 0x40;

void initDisplay() {
  Wire.begin();

  // Display off
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0xAE);
  Wire.endTransmission();

  // Set memory addressing mode to horizontal
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0x20);
  Wire.write(0x00);
  Wire.endTransmission();

  // Display on
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0xAF);
  Wire.endTransmission();

  clearDisplay();
}

void clearDisplay() {
  for (uint8_t page = 0; page < 8; page++) {
    clearPage(page);
  }
}

void clearPage(uint8_t page) {
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0xB0 + page);
  Wire.endTransmission();

  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0x10);
  Wire.endTransmission();

  for (int i = 0; i < 128; i++) {
    Wire.beginTransmission(DISPLAY_ADDR);
    Wire.write(DATA);
    Wire.write(0x00);
    Wire.endTransmission();
  }
}

void displayText(String text, uint8_t page, uint8_t startCol) {
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0xB0 + page);
  Wire.endTransmission();

  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(startCol & 0x0F);
  Wire.endTransmission();

  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(COMMAND);
  Wire.write(0x10 + (startCol >> 4));
  Wire.endTransmission();

  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    // Degree symbol UTF encoded is 2 bytes so we have to check this indivitually
    if (i + 1 < text.length() && (uint8_t)c == 0xC2 && (uint8_t)text.charAt(i + 1) == 0xB0) {
      const uint8_t* degree = getDegree();
      for (int j = 0; j < 8; j++) {
        uint8_t pixelData = pgm_read_byte(&degree[j]);
        Wire.beginTransmission(DISPLAY_ADDR);
        Wire.write(DATA);
        Wire.write(pixelData);
        Wire.endTransmission();
      }
      // We already checked the next bit so advance i
      i++;
      continue;
    }

    const uint8_t* bitmap = getCharBitmap(c);

    for (int j = 0; j < 8; j++) {
      uint8_t pixelData = pgm_read_byte(&bitmap[j]);
      Wire.beginTransmission(DISPLAY_ADDR);
      Wire.write(DATA);
      Wire.write(pixelData);
      Wire.endTransmission();
    }
  }
}

void displayNumber(uint32_t number, uint8_t page, uint8_t startCol) {
  String numStr = String(number);
  displayText(numStr, page, startCol);
}