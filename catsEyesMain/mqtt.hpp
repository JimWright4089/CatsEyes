
#ifndef MQTT_HPP
#define MQTT_HPP

#include "stdint.h"

void mqttInit();
void mqttRun();
void reportEyeChange(uint8_t number, uint32_t color, uint8_t state, uint32_t lastCount, uint32_t sinceOpen);

#endif
