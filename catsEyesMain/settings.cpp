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
const uint16_t SIZE_OF_DATA_FLASH = SIZE_OF_FLASH - SIZE_OF_CRC16;
const uint16_t SIZE_OF_IP4_ADDRESS = 4;
const uint16_t SIZE_OF_CRC = 2;

const uint16_t LOC_SSID = 0;
const uint16_t LOC_PASSWORD = LOC_SSID + SIZE_OF_STRING;
const uint16_t LOC_IP4_ADDRESS_OF_MQTT = LOC_PASSWORD + SIZE_OF_STRING;

const uint8_t* DEFAULT_SSID = (const uint8_t*)"provisioner";
const uint8_t* DEFAULT_PASSWORD = (const uint8_t*)"provisioner-pwd";
const uint8_t* DEFAULT_IP4 = (const uint8_t*)"prov";
bool lEepromGood=false;
uint8_t lEepromBlock[SIZE_OF_FLASH];

uint8_t lSsid[SIZE_OF_STRING];
uint8_t lPassword[SIZE_OF_STRING];
uint8_t lIp4[SIZE_OF_IP4_ADDRESS];
uint8_t lLedState = STATE_RUN;

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
      memset(lEepromBlock,0x00,SIZE_OF_FLASH);

      setSsid((uint8_t*)DEFAULT_SSID);
      setPassword((uint8_t*)DEFAULT_PASSWORD);
      setIp4((uint8_t*)DEFAULT_IP4);
      commitEeprom();
    }
    else
    {
      memcpy(lSsid,&lEepromBlock[LOC_SSID],SIZE_OF_STRING);
      memcpy(lPassword,&lEepromBlock[LOC_PASSWORD],SIZE_OF_STRING);
      memcpy(lIp4,&lEepromBlock[LOC_IP4_ADDRESS_OF_MQTT],SIZE_OF_IP4_ADDRESS);
    }
  }

  log_i("SSID:    [%s]",lSsid);
  log_i("Password:[%s]",lPassword);
}

void setSsid(uint8_t* ssid)
{
  log_i("Set SSID");
  memcpy(lSsid,ssid,SIZE_OF_STRING);
  memcpy(&lEepromBlock[LOC_SSID],ssid,SIZE_OF_STRING);
}

void setPassword(uint8_t* password)
{
  log_i("Set Password");
  memcpy(lPassword,password,SIZE_OF_STRING);
  memcpy(&lEepromBlock[LOC_PASSWORD],password,SIZE_OF_STRING);
}

void setIp4(uint8_t* ip4)
{
  log_i("Set IP4");
  memcpy(lIp4,ip4,SIZE_OF_IP4_ADDRESS);
  memcpy(&lEepromBlock[LOC_IP4_ADDRESS_OF_MQTT],ip4,SIZE_OF_IP4_ADDRESS);
}

void commitEeprom()
{
  log_i("commit to EERPOM");
  uint16_t calcedCrc = calcCRC16(lEepromBlock, SIZE_OF_DATA_FLASH);
  memcpy(&lEepromBlock[SIZE_OF_DATA_FLASH],&calcedCrc,SIZE_OF_CRC);
  uint16_t length = EEPROM.writeBytes(0, lEepromBlock, SIZE_OF_FLASH);
  EEPROM.commit();
}

uint8_t* getSsid()
{
  return lSsid;
}

uint8_t* getPassword()
{
  return lPassword;
}

uint8_t getState()
{
  return lLedState;
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
    }
    else
    {
      sendError("unknowncmd",message);
    }
  }
  else
  {
    sendError("nocmd",message);
  }
}

void setField(String field, JSONVar data)
{
  if(true == field.equals("state"))
  {
    setState((String)data);
    return;
  }
  sendError("badfield",(char*)field.c_str());
}

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