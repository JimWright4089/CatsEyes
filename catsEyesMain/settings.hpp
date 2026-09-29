
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Arduino_JSON.h>
#include "stdint.h"

const uint8_t STATE_RUN = 1;
const uint8_t STATE_TEST = 2;
const uint8_t STATE_OFF = 3;

void initSettings();

void setSsid(uint8_t* ssid);
void setPassword(uint8_t* password);
void setIp4(uint8_t* ip4);
void commitEeprom();
uint8_t getState();
void setState(String state);
uint8_t* getSsid();
uint8_t* getPassword();



void setSetting(char* message);
void setField(String field, JSONVar data);

#endif
