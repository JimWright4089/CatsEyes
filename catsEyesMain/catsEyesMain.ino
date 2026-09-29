
#include "settings.hpp"
#include "versions.hpp"
#include "neoPixels.hpp"
#include "ota.hpp"
#include "mqtt.hpp"
#include "stopWatch.hpp"

void runEyes();
void runTestEyes();
uint32_t getEyeColor();
void clearEyes();

int gEyeState[NUM_OF_EYES];
uint32_t gEyeColor[NUM_OF_EYES];
bool gHasBlinked[NUM_OF_EYES];
StopWatch blinkWatch[NUM_OF_EYES];
uint32_t gEyeStatcount[NUM_OF_EYES];
uint32_t gEyeStatCountFromOpen[NUM_OF_EYES];
uint8_t gLastState=STATE_RUN;

StopWatch eyesWork;
StopWatch eyesTest;

void setup() 
{
  randomSeed(analogRead(A0));
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  initPixels();
  setBoardPixel(BOARD_COLOR);
  showBoardPixel();

  Serial.println("Starting Cats Eyes");
  Serial.print("Version:");
  Serial.print(APP_VERSION_MAJOR);
  Serial.print(".");
  Serial.println(APP_VERSION_MINOR);

  delay(1500);
  log_i("Starting Cats Eyes");
  delay(1000);

  initSettings();
  wifiInit();
  otaInit();
  mqttInit();

  clearEyes();
  showEyes();
  eyesTest.set_time(10000);
}

void loop() 
{
  otaRun();
  mqttRun();
  if(true == eyesWork.is_expired())
  {
    if(STATE_RUN == getState())
    {
      if(STATE_RUN != gLastState)
      {
        clearEyes();
      }
      runEyes();
    }

    if(STATE_TEST == getState())
    {
      if(true == eyesTest.is_expired())
      {
        runTestEyes();
        eyesTest.reset();
      }
    }

    if(STATE_OFF == getState())
    {
      clearEyes();
    }

    gLastState=getState();
    showEyes();
    eyesWork.reset();
  }
}

uint32_t getEyeColor()
{
  uint8_t color = random(0, 100);

  if(color < 5)  // 5
  {
    return BLUE;
  }

  if(color < 12) // 7
  {
    return MAGENTA;
  }

  if(color < 24) // 12
  {
    return CYAN;
  }

  if(color < 39) // 15
  {
    return GREEN;
  }

  if(color < 56) // 17
  {
    return YELLOW;
  }

  if(color < 76) // 20
  {
    return WHITE;
  }

  return RED;

}

void runEyes()
{
  for(int i=0;i<NUM_OF_EYES;i++)
  {
    gEyeStatcount[i]++;
    gEyeStatCountFromOpen[i]++;

    //---------------------------------------------------------------
    //
    //  Reset the blink is a long time has passed
    //
    //---------------------------------------------------------------
    if((true == gHasBlinked[i])&&(true == blinkWatch[i].is_expired()))
    {
      gHasBlinked[i] = false;
    }


    //---------------------------------------------------------------
    //
    //  Should I open the eye?
    //
    //---------------------------------------------------------------
    if(EYE_CLOSED == gEyeState[i])
    {
      if(0 == random(0, 50))
      {
        gEyeState[i] = EYE_OPEN;
        gHasBlinked[i] = false;
        gEyeColor[i] = getEyeColor();
        setEyeColor(i, gEyeColor[i]);
        reportEyeChange(i, gEyeColor[i], gEyeState[i], gEyeStatcount[i],0);
        gEyeStatcount[i] = 0;
        gEyeStatCountFromOpen[i] = 0;
      }
    }
    else
    {
      //---------------------------------------------------------------
      //
      //  Should I close the eye?
      //
      //---------------------------------------------------------------
      if(EYE_OPEN == gEyeState[i])
      {
        if(0 == random(0, 100))
        {
          gEyeState[i] = EYE_CLOSED;
          gEyeColor[i] = BLACK;
          setEyeColor(i, gEyeColor[i]);
          reportEyeChange(i, gEyeColor[i], gEyeState[i], gEyeStatcount[i],gEyeStatCountFromOpen[i]);
          gEyeStatcount[i] = 0;
        }
        else
        {
          //---------------------------------------------------------------
          //
          //  Eye is still open, should I blink it>
          //
          //---------------------------------------------------------------
          if(false == gHasBlinked[i])
          {
            if(0 == random(0, 50))
            {
              gEyeState[i] = EYE_BLINK;
              setEyeColor(i, BLACK);
              blinkWatch[i].set_time(random(800, 3000));
              blinkWatch[i].reset();
              reportEyeChange(i, gEyeColor[i], gEyeState[i], gEyeStatcount[i],gEyeStatCountFromOpen[i]);
              gEyeStatcount[i] = 0;
            }
          }
        }
      }
      else
      {
        //---------------------------------------------------------------
        //
        //  Should I stop blinking
        //
        //---------------------------------------------------------------
        if(EYE_BLINK == gEyeState[i])
        {
          if(true == blinkWatch[i].is_expired())
          {
            setEyeColor(i, gEyeColor[i]);
            gEyeState[i] = EYE_OPEN;
            gHasBlinked[i] = true;
            reportEyeChange(i, gEyeColor[i], EYE_BLINK_OFF, gEyeStatcount[i],gEyeStatCountFromOpen[i]);
            gEyeStatcount[i] = 0;
            blinkWatch[i].set_time(70000);
            blinkWatch[i].reset();
          }
        }
      }
    }
  }
}

void runTestEyes()
{
  uint32_t color = BLACK;

  switch(gEyeColor[0])
  {
    case BLACK:
      color=RED;
      break;
    case RED:
      color=GREEN;
      break;
    case GREEN:
      color=BLUE;
      break;
    case BLUE:
      color=YELLOW;
      break;
    case YELLOW:
      color=MAGENTA;
      break;
    case MAGENTA:
      color=CYAN;
      break;
    case CYAN:
      color=WHITE;
      break;
    default:
      color=BLACK;
      break;
  }

  for(int i=0;i<NUM_OF_EYES;i++)
  {
    gEyeColor[i] = color;
    setEyeColor(i, gEyeColor[i]);
    reportEyeChange(i, gEyeColor[i], EYE_TEST, gEyeStatcount[i],gEyeStatCountFromOpen[i]);
  }
}

void clearEyes()
{
  for(int i=0;i<NUM_OF_EYES;i++)
  {
    gEyeState[i] = EYE_CLOSED;
    gEyeColor[i] = BLACK;
    gHasBlinked[i] = false;
    gEyeStatcount[i] = 0;
    gEyeStatCountFromOpen[i] = 0;
    setEyeColor(i, gEyeColor[i]);
  }
}
