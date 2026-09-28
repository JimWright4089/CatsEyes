

#include <Adafruit_NeoPixel.h>
#include "neoPixels.hpp"

#define BOARD_PIN        9
#define BOARD_NUM_PIXELS 1

#define EYE_PIN          8
#define EYE_NUM_PIXELS  (NUM_OF_EYES * 2)
#define PIXELS_PER_EYE   2

Adafruit_NeoPixel boardPixel(BOARD_NUM_PIXELS, BOARD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyePixels(EYE_NUM_PIXELS, EYE_PIN, NEO_GRB + NEO_KHZ800);

void initPixels()
{
  boardPixel.begin();
  eyePixels.begin();
}

void setBoardPixel(uint32_t color)
{
  boardPixel.setPixelColor(0, color);
}

void showBoardPixel()
{
  boardPixel.show();  
}

void setEyeColor(uint16_t number, uint32_t color)
{
  int leftEyeNumber = number*PIXELS_PER_EYE;
  int rightEyeNumber = leftEyeNumber + 1;

  eyePixels.setPixelColor(leftEyeNumber, color);
  eyePixels.setPixelColor(rightEyeNumber, color);
}

void showEyes()
{
  eyePixels.show();  
}

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