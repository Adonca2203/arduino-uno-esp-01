#ifndef FONTS_H
#define FONTS_H

#include <Arduino.h>
#include <avr/pgmspace.h>

const uint8_t* getCharBitmap(char c);
const uint8_t* getDegree();
char digitToChar(uint8_t digit);
int numberToDigits(uint32_t num, uint8_t digits[], int maxDigits);

#endif