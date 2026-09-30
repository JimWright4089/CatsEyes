
#ifndef MQTT_HPP
#define MQTT_HPP

#include "stdint.h"

const uint16_t MAX_MQTT_PACKET = 400;

void mqttInit();
void mqttRun();
void reportEyeChange(uint8_t number, uint32_t color, uint8_t state, uint32_t lastCount, uint32_t sinceOpen);
void sendError(char* command, char *message);
void sendSettings(char* setting, char *value);
void sendSettings(char* setting, uint16_t value);
void sendSettings(char* setting, uint32_t value);

#endif
