//----------------------------------------------------------------------------
//
//  $Workfile: Settings.hpp
//
//  Copywrite:
//      
//
//  Notes:
//     This is the code for coneecting to things with a userid and password
//
//----------------------------------------------------------------------------
#include <Arduino_JSON.h>
#include "mqtt.hpp"
#include "settings.hpp"
#include "memory.h"
#include "EEPROM.h"
#include "CRC16.h"
#include "CRC.h"

const uint16_t SIZE_OF_FLASH = 1024;
const uint16_t SIZE_OF_STRING = 128;
const uint16_t SIZE_OF_CRC16  =   2;
const uint16_t SIZE_OF_COUNT  =   2;
const uint32_t SIZE_OF_TIME   =   4;
const uint16_t SIZE_OF_DATA_FLASH = SIZE_OF_FLASH - SIZE_OF_CRC16;
const uint16_t SIZE_OF_BROKER_ADDRESS = 16;
const uint16_t SIZE_OF_CRC = 2;

const uint16_t LOC_SSID           = 0;
const uint16_t LOC_PASSWORD       = LOC_SSID + SIZE_OF_STRING;
const uint16_t LOC_BROKER_ADDRESS_OF_MQTT = LOC_PASSWORD + SIZE_OF_STRING;
const uint16_t LOC_EYE_OPEN       = LOC_BROKER_ADDRESS_OF_MQTT + SIZE_OF_BROKER_ADDRESS;
const uint16_t LOC_EYE_CLOSE      = LOC_EYE_OPEN + SIZE_OF_COUNT;
const uint16_t LOC_EYE_BLINK      = LOC_EYE_CLOSE + SIZE_OF_COUNT;
const uint16_t LOC_EYE_BLINK_TIME = LOC_EYE_BLINK + SIZE_OF_COUNT;

const char* DEFAULT_SSID = "provisioner";
const char* DEFAULT_PASSWORD = "provisioner-pwd";
const char* DEFAULT_IP4 = "192.168.10.10";
const uint16_t DEFAULT_EYE_OPEN   = 100;
const uint16_t DEFAULT_EYE_CLOSE  = 100;
const uint16_t DEFAULT_EYE_BLINK  = 50;
const uint32_t DEFAULT_EYE_BLINK_TIME = 20000;

bool lEepromGood=false;
uint8_t lEepromBlock[SIZE_OF_FLASH];

char lSsid[SIZE_OF_STRING];
char lPassword[SIZE_OF_STRING];
char lBroker[SIZE_OF_BROKER_ADDRESS];
uint8_t lLedState = STATE_RUN;
uint16_t lEyeOpenCount        = 100;
uint16_t lEyeCloseCount       = 100;
uint16_t lEyeBlinkCount       = 50;
uint32_t lEyeBlinkLockoutTime = 20000;

void initSettings()
{
  delay(1);
  log_i("Reading EEPROM");

  if (!EEPROM.begin(SIZE_OF_FLASH)) {
    log_e("Can't open the eeprom");
  }
  else
  {
    lEepromGood=true;
    int length = EEPROM.readBytes(0, lEepromBlock, SIZE_OF_FLASH);

    log_d("Length returned=%d",length);
    log_buf_d(lEepromBlock, SIZE_OF_FLASH);

    uint16_t calcedCrc = calcCRC16(lEepromBlock, SIZE_OF_DATA_FLASH);
    uint16_t readCrc = 0;
    memcpy(&readCrc,&lEepromBlock[SIZE_OF_DATA_FLASH],SIZE_OF_CRC);
    log_d("Calc'd CRC=%x, read CRC=%x",calcedCrc,readCrc);

    if(calcedCrc != readCrc)
    {
      log_e("Error in the CRC Re-initing");
      log_e("Calc'd CRC=%x, read CRC=%x",calcedCrc,readCrc);
      clearSettings();
    }
    else
    {
      memcpy(lSsid,&lEepromBlock[LOC_SSID],SIZE_OF_STRING);
      memcpy(lPassword,&lEepromBlock[LOC_PASSWORD],SIZE_OF_STRING);
      memcpy(lBroker,&lEepromBlock[LOC_BROKER_ADDRESS_OF_MQTT],SIZE_OF_BROKER_ADDRESS);

      memcpy(&lEyeOpenCount,&lEepromBlock[LOC_EYE_OPEN],SIZE_OF_COUNT);
      memcpy(&lEyeCloseCount,&lEepromBlock[LOC_EYE_CLOSE],SIZE_OF_COUNT);
      memcpy(&lEyeBlinkCount,&lEepromBlock[LOC_EYE_BLINK],SIZE_OF_COUNT);
      memcpy(&lEyeBlinkLockoutTime,&lEepromBlock[LOC_EYE_BLINK_TIME],SIZE_OF_TIME);
    }
  }

  log_i("ssid:      [%s]",lSsid);
  log_i("password:  [%s]",lPassword);
  log_i("broker:    [%s]",lBroker);

  log_i("eyeopen:   [%d]",lEyeOpenCount);
  log_i("eyeclose:  [%d]",lEyeCloseCount);
  log_i("eyeblink:  [%d]",lEyeBlinkCount);
  log_i("blinktime: [%d]",lEyeBlinkLockoutTime);
}

void clearSettings()
{
  log_w("Clearing Settings");

  memset(lEepromBlock,0x00,SIZE_OF_FLASH);

  setSsid((char*)DEFAULT_SSID);
  setPassword((char*)DEFAULT_PASSWORD);
  setBroker((char*)DEFAULT_IP4);

  setEyeOpenCount(DEFAULT_EYE_OPEN);
  setEyeCloseCount(DEFAULT_EYE_CLOSE);
  setEyeBlinkCount(DEFAULT_EYE_BLINK);
  setEyeBlinkLockoutTime(DEFAULT_EYE_BLINK_TIME);

  commitEeprom();
}

void dumpSettings()
{
  sendSettings("ssid",lSsid);
  sendSettings("password",lPassword);
  sendSettings("broker",lBroker);

  sendSettings("eyeopen",lEyeOpenCount);
  sendSettings("eyeclose",lEyeCloseCount);
  sendSettings("eyeblink",lEyeBlinkCount);
  sendSettings("blinktime",lEyeBlinkLockoutTime);
}

void commitEeprom()
{
  log_i("commit to EERPOM");
  uint16_t calcedCrc = calcCRC16(lEepromBlock, SIZE_OF_DATA_FLASH);
  memcpy(&lEepromBlock[SIZE_OF_DATA_FLASH],&calcedCrc,SIZE_OF_CRC);
  EEPROM.writeBytes(0, lEepromBlock, SIZE_OF_FLASH);
  EEPROM.commit();
}

void setSetting(char* message)
{
  JSONVar myObject = JSON.parse(message);

  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") 
  {
    sendError("parseerror",message);
    return;
  }

  if (myObject.hasOwnProperty("cmd")) 
  {
    Serial.print("myObject[\"cmd\"] = ");
    Serial.println((const char*) myObject["cmd"]);

    String cmd = (String)myObject["cmd"];

    if(true == cmd.equals("set"))
    {
      if ((myObject.hasOwnProperty("field")) && (myObject.hasOwnProperty("data")))
      {
        Serial.print("myObject[\"field\"] = ");
        Serial.println((const char*) myObject["field"]);
        setField((String) myObject["field"], myObject["data"]);
      }
      else
      {
        sendError("nosetfielddata",message);
      }
      return;
    }

    if(true == cmd.equals("reboot"))
    {
      ESP.restart();
      return;
    }

    if(true == cmd.equals("clearsettings"))
    {
      clearSettings();
      return;
    }

    if(true == cmd.equals("dumpsettings"))
    {
      dumpSettings();
      return;
    }

    sendError("unknowncmd",message);
    return;
  }
  sendError("nocmd",message);

}

void setField(String field, JSONVar data)
{
  if(true == field.equals("state"))
  {
    setState((String)data);
    return;
  }

  if(true == field.equals("ssid"))
  {
    setSsid((char*)((String)data).c_str());
    commitEeprom();
    return;
  }

  if(true == field.equals("password"))
  {
    setPassword((char*)((String)data).c_str());
    commitEeprom();
    return;
  }

  if(true == field.equals("broker"))
  {
    setBroker((char*)((String)data).c_str());
    commitEeprom();
    return;
  }

  if(true == field.equals("eyeopen"))
  {
    setEyeOpenCount(((unsigned short)data));
    commitEeprom();
    return;
  }

  if(true == field.equals("eyeclose"))
  {
    setEyeCloseCount(((unsigned short)data));
    commitEeprom();
    return;
  }

  if(true == field.equals("eyeblink"))
  {
    setEyeBlinkCount(((unsigned short)data));
    commitEeprom();
    return;
  }

  if(true == field.equals("blinktime"))
  {
    setEyeBlinkLockoutTime(((unsigned long)data));
    commitEeprom();
    return;
  }

  sendError("badfield",(char*)field.c_str());
}

//-------------------------------------------------------------------------
//   Getter/Setters
//  
//-------------------------------------------------------------------------

void setState(String state)
{
  if(true == state.equals("off"))
  {
    lLedState = STATE_OFF;  
    return;
  }
  if(true == state.equals("run"))
  {
    lLedState = STATE_RUN;  
    return;
  }
  if(true == state.equals("test"))
  {
    lLedState = STATE_TEST;  
    return;
  }
  sendError("badstate",(char*)state.c_str());
}

uint8_t getState()
{
  return lLedState;
}

void setSsid(char* ssid)
{
  log_i("Set SSID");
  memcpy(lSsid,ssid,SIZE_OF_STRING);
  memcpy(&lEepromBlock[LOC_SSID],ssid,SIZE_OF_STRING);
}

char* getSsid()
{
  return lSsid;
}

void setPassword(char* password)
{
  log_i("Set Password");
  memcpy(lPassword,password,SIZE_OF_STRING);
  memcpy(&lEepromBlock[LOC_PASSWORD],password,SIZE_OF_STRING);
}

char* getPassword()
{
  return lPassword;
}

void setBroker(char* broker)
{
  log_i("Set Broker");
  memcpy(lBroker,broker,SIZE_OF_BROKER_ADDRESS);
  memcpy(&lEepromBlock[LOC_BROKER_ADDRESS_OF_MQTT],broker,SIZE_OF_BROKER_ADDRESS);
}

char* getBroker()
{
  return lBroker;
}

void setEyeOpenCount(uint16_t count)
{
  log_i("Set Eye Open");
  lEyeOpenCount = count;
  memcpy(&lEepromBlock[LOC_EYE_OPEN],&lEyeOpenCount,SIZE_OF_COUNT);
}

uint16_t getEyeOpenCount()
{
  return lEyeOpenCount;
}

void setEyeCloseCount(uint16_t count)
{
  log_i("Set Eye Close");
  lEyeCloseCount = count;
  memcpy(&lEepromBlock[LOC_EYE_CLOSE],&lEyeCloseCount,SIZE_OF_COUNT);
}

uint16_t getEyeCloseCount()
{
  return lEyeCloseCount;
}

void setEyeBlinkCount(uint16_t count)
{
  log_i("Set Eye Blink");
  lEyeBlinkCount = count;
  memcpy(&lEepromBlock[LOC_EYE_BLINK],&lEyeBlinkCount,SIZE_OF_COUNT);
}

uint16_t getEyeBlinkCount()
{
  return lEyeBlinkCount;
}

void setEyeBlinkLockoutTime(uint32_t count)
{
  log_i("Set Eye Blink Time");
  lEyeBlinkLockoutTime = count;
  memcpy(&lEepromBlock[LOC_EYE_BLINK_TIME],&lEyeBlinkLockoutTime,SIZE_OF_TIME);
}

uint32_t getEyeBlinkLockoutTime()
{
  return lEyeBlinkLockoutTime;
}
