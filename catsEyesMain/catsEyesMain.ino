//----------------------------------------------------------------------------
//
//  Workfile: catsEyesMain.ino
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     Main application entry point and controller logic for the Cats Eyes device.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
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
StopWatch lWifiSetup(10000);

//----------------------------------------------------------------------------
//  Purpose:
//   LED Task
//
//  Notes:
//
//----------------------------------------------------------------------------
void ledTask(void * pvParameters) 
{
  while(true) 
  {
    setBoardPixel(BOARD_COLOR);
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
}


//----------------------------------------------------------------------------
//  Purpose:
//   Initialize the device, sensors, and network services.
//
//  Notes:
//
//----------------------------------------------------------------------------
void setup() 
{
  randomSeed(analogRead(A0));
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  initPixels();
  setBoardPixel(BOARD_COLOR);

  Serial.println("Starting Cats Eyes");
  Serial.print("Version:");
  Serial.print(APP_VERSION_MAJOR);
  Serial.print(".");
  Serial.println(APP_VERSION_MINOR);

  delay(1500);
  log_i("Starting Cats Eyes");
  delay(1000);

  xTaskCreatePinnedToCore(
      ledTask,   /* Function to implement the task */
      "LedTask",     /* Name of the task */
      10000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      1,           /* Priority of the task */
      NULL,        /* Task handle */
      0            /* Core where the task should run (0 or 1) */
    );  

  initSettings();
  wifiInit();
  mqttInit();

  clearEyes();
  showEyes();
  eyesTest.set_time(10000);
}

//----------------------------------------------------------------------------
//  Purpose:
//   Run the main control loop for OTA, MQTT, and eye behavior.
//
//  Notes:
//
//----------------------------------------------------------------------------
void loop() 
{
  if(true == lWifiSetup.is_expired())
  {
    if(false == isWifiGood())
    {
      log_e("Trying to setup WIFI");
      wifiInit();
    }
    if(false == isMqttGood())
    {
      log_e("Trying to setup MQTT");
      mqttInit();
    }
    lWifiSetup.set_time(10000);
    lWifiSetup.reset();
  }

  otaRun();
  mqttRun();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Return a random eye color from the configured palette.
//
//  Notes:
//
//----------------------------------------------------------------------------
uint32_t getEyeColor()
{
  uint8_t color = random(0, 100);

  if(color < 3)  // 5
  {
    return BLUE;
  }

  if(color < 6) // 7
  {
    return MAGENTA;
  }

  if(color < 15) // 9
  {
    return CYAN;
  }

  if(color < 27) // 12
  {
    return GREEN;
  }

  if(color < 42) // 15
  {
    return YELLOW;
  }

  if(color < 56) // 17
  {
    return ORANGE;
  }

  if(color < 76) // 20
  {
    return WHITE;
  }

  return RED;

}

//----------------------------------------------------------------------------
//  Purpose:
//   Update the state of each eye according to open, close, and blink rules.
//
//  Notes:
//
//----------------------------------------------------------------------------
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
      if(0 == random(0, getEyeOpenCount()))
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
        if(0 == random(0, getEyeCloseCount()))
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
            if(0 == random(0, getEyeBlinkCount()))
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
            blinkWatch[i].set_time(getEyeBlinkLockoutTime());
            blinkWatch[i].reset();
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
//  Purpose:
//   Run the test sequence that cycles through the eye colors.
//
//  Notes:
//
//----------------------------------------------------------------------------
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
      color=ORANGE;
      break;
    case ORANGE:
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

//----------------------------------------------------------------------------
//  Purpose:
//   Clear all eye state and reset the display to the off condition.
//
//  Notes:
//
//----------------------------------------------------------------------------
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
