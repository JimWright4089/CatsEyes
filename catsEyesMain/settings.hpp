
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Arduino_JSON.h>
#include "stdint.h"

const uint8_t STATE_RUN = 1;
const uint8_t STATE_TEST = 2;
const uint8_t STATE_OFF = 3;

void initSettings();

void clearSettings();
void dumpSettings();
void setSsid(char* ssid);
void setPassword(char* password);
void setBroker(char* ip4);
void commitEeprom();
uint8_t getState();
void setState(String state);
char* getSsid();
char* getPassword();
char* getBroker();

void setSetting(char* message);
void setField(String field, JSONVar data);

#endif
