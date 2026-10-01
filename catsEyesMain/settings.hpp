//----------------------------------------------------------------------------
//
//  Workfile: settings.hpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     EEPROM and configuration interface definitions for the Cats Eyes device.
//
//----------------------------------------------------------------------------
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include <Arduino_JSON.h>
#include "stdint.h"

const uint8_t STATE_RUN = 1;
const uint8_t STATE_TEST = 2;
const uint8_t STATE_OFF = 3;

void initSettings();

void clearSettings();
void dumpSettings();
void commitEeprom();
void setSetting(char* message);
void setField(String field, JSONVar data);

void setState(String state);
uint8_t getState();
void setSsid(char* ssid);
char* getSsid();
void setPassword(char* password);
char* getPassword();
void setBroker(char* ip4);
char* getBroker();

void setEyeOpenCount(uint16_t count);
uint16_t getEyeOpenCount();
void setEyeCloseCount(uint16_t count);
uint16_t getEyeCloseCount();
void setEyeBlinkCount(uint16_t count);
uint16_t getEyeBlinkCount();
void setEyeBlinkLockoutTime(uint32_t time);
uint32_t getEyeBlinkLockoutTime();

#endif
