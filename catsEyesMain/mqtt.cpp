
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <string.h>
#include "mqtt.hpp"
#include "stopWatch.hpp"
#include "neoPixels.hpp"

void sendPing();


StopWatch pollWatch(500);
StopWatch pingWatch(60000);

const char broker[] = "192.168.143.40";
int        port     = 1883;
const char topic[]  = "arduino/simple";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
bool mqttConnected=false;

//{ "cmd": "ping", "value":6325}

const char healthTopic[] = "cats-eyes/health";
const char eyeChaangeTopic[] = "cats-eyes/status";
int count=0;
int gPingCount = 0;

void mqttInit()
{
  mqttClient.setId("clientId");
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  }
  else
  {
    mqttConnected=true;
  }

  if(true==mqttConnected)
  {
    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
  }
  sendPing();
}


void mqttRun()
{
  if(false == mqttConnected)
  {
    return;
  }

  if(pollWatch.is_expired())
  {
    log_d("mqtt poll");
    mqttClient.poll();
    pollWatch.reset();
  }

  if(pingWatch.is_expired())
  {
    sendPing();
  }
}

void reportEyeChange(uint8_t number, uint32_t color, uint8_t state, uint32_t lastCount, uint32_t sinceOpen)
{
  if(false == mqttConnected)
  {
    return;
  }

  mqttClient.beginMessage(eyeChaangeTopic);
  mqttClient.print("{ \"number\": ");
  mqttClient.print(number);
  mqttClient.print(", \"color\": \"");
  mqttClient.print(getColorName(color));
  mqttClient.print("\", \"state\": \"");
  mqttClient.print(getStateName(state));
  mqttClient.print("\", \"laststatetime\": ");
  mqttClient.print(lastCount);
  mqttClient.print(", \"sinceopen\": ");
  mqttClient.print(sinceOpen);
  mqttClient.print("}");
  mqttClient.endMessage();

  log_i("number=%d color=%s state=%s lastStateTime=%d timeFromOpen=%d",number,getColorName(color),getStateName(state),lastCount,sinceOpen);
}

void sendPing()
{
  if(false == mqttConnected)
  {
    return;
  }
  mqttClient.beginMessage(healthTopic);
  mqttClient.print("{ \"cmd\": \"ping\", \"value\":");
  mqttClient.print(gPingCount);
  mqttClient.print("}");
  mqttClient.endMessage();

  log_d("Sending ping=%d",gPingCount);
  pingWatch.reset();
  gPingCount++;
}
