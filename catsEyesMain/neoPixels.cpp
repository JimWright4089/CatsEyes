//----------------------------------------------------------------------------
//
//  Workfile: neoPixels.cpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     Implementation of the board and eye NeoPixel control helpers.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#include "neoPixels.hpp"

#define BOARD_PIN        9
#define BOARD_NUM_PIXELS 1

#define EYE_PIN          8
#define EYE_NUM_PIXELS  (NUM_OF_EYES * 2)
#define PIXELS_PER_EYE   2

Adafruit_NeoPixel boardPixel(BOARD_NUM_PIXELS, BOARD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyePixels(EYE_NUM_PIXELS, EYE_PIN, NEO_GRB + NEO_KHZ800);

//----------------------------------------------------------------------------
//  Purpose:
//   Initialize the board and eye pixel objects.
//
//  Notes:
//
//----------------------------------------------------------------------------
void initPixels()
{
  boardPixel.begin();
  eyePixels.begin();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Set the board status pixel to the given color.
//
//  Notes:
//
//----------------------------------------------------------------------------
void setBoardPixel(uint32_t color)
{
  boardPixel.setPixelColor(0, color);
}

//----------------------------------------------------------------------------
//  Purpose:
//   Refresh the board status pixel on the output bus.
//
//  Notes:
//
//----------------------------------------------------------------------------
void showBoardPixel()
{
  boardPixel.show();  
}

//----------------------------------------------------------------------------
//  Purpose:
//   Set both pixels for the specified eye to the given color.
//
//  Notes:
//
//----------------------------------------------------------------------------
void setEyeColor(uint16_t number, uint32_t color)
{
  int leftEyeNumber = number*PIXELS_PER_EYE;
  int rightEyeNumber = leftEyeNumber + 1;

  eyePixels.setPixelColor(leftEyeNumber, color);
  eyePixels.setPixelColor(rightEyeNumber, color);
}

//----------------------------------------------------------------------------
//  Purpose:
//   Refresh the eye pixel strip on the output bus.
//
//  Notes:
//
//----------------------------------------------------------------------------
void showEyes()
{
  eyePixels.show();  
}

//----------------------------------------------------------------------------
//  Purpose:
//   Convert a numeric color value to a human-readable name.
//
//  Notes:
//
//----------------------------------------------------------------------------
const char* getColorName(uint32_t color)
{
  switch(color)
  {
    case BLACK:
      return "black";
    case RED:
      return "red";
    case GREEN:
      return "green";
    case BLUE:
      return "blue";
    case YELLOW:
      return "yellow";
    case MAGENTA:
      return "magenta";
    case CYAN:
      return "cyan";
    case WHITE:
      return "white";
    default:
      return "unknown";
  }
}

//----------------------------------------------------------------------------
//  Purpose:
//   Convert a numeric eye state value to a readable label.
//
//  Notes:
//
//----------------------------------------------------------------------------
const char* getStateName(uint8_t state)
{
  switch(state)
  {
    case EYE_CLOSED:
      return "closing";
    case EYE_OPEN:
      return "opening";
    case EYE_BLINK:
      return "start blinking";
    case EYE_BLINK_OFF:
      return "end blinking";
    case EYE_TEST:
      return "testing";
    default:
      return "unknown";
  }
}