#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

extern const int DISPLAY_ADDR;
extern const uint8_t COMMAND;
extern const uint8_t DATA;

void initDisplay();
void clearDisplay();
void clearPage(uint8_t page);
void displayText(String text, uint8_t page, uint8_t startCol);
void displayNumber(uint32_t number, uint8_t page, uint8_t startCol);

#endif