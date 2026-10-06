//----------------------------------------------------------------------------
//
//  Workfile: neoPixels.hpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     Declaration of NeoPixel helpers for the board and eye arrays.
//
//----------------------------------------------------------------------------
#ifndef NEO_PIXEL_H
#define NEO_PIXEL_H

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#ifdef ARDUINO_ARDUINO_NESSO_N1
#include <Arduino_Nesso_N1.h>
#endif

#include <Adafruit_NeoPixel.h>

#define BLACK   0x00000000
#define RED     0x001E0000
#define GREEN   0x00001E00
#define BLUE    0x0000001E
#define YELLOW  0x001E1E00
#define MAGENTA 0x001E001E
#define CYAN    0x00001E1E
#define ORANGE  0x001E0700
#define WHITE   0x001E1E1E

#define NUM_OF_EYES 25

#define EYE_CLOSED 0
#define EYE_OPEN   1
#define EYE_BLINK  2
#define EYE_BLINK_OFF 3
#define EYE_TEST 4

void initPixels();
void setBoardPixel(uint32_t color);
void setEyeColor(uint16_t number, uint32_t color);
void showEyes();
const char* getColorName(uint32_t color);
char getColorLetter(uint32_t color);
const char* getStateName(uint8_t state);

#ifdef ARDUINO_ARDUINO_NESSO_N1
int getDisplayColor(uint32_t color);
#endif

#endif NEO_PIXEL_H
