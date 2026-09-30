//----------------------------------------------------------------------------
//
//  Workfile: mqtt.cpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     All of the MQTT handling code 
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <string.h>
#include "mqtt.hpp"
#include "stopWatch.hpp"
#include "neoPixels.hpp"
#include "settings.hpp"
#include "versions.hpp"

void sendPing();
void onMqttMessage(int messageSize);

StopWatch pollWatch(500);
StopWatch pingWatch(60000);
StopWatch fullStatus(300000);

int        port     = 1883;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
bool mqttConnected=false;

const char healthTopic[] = "cats-eyes/health";
const char eyeChaangeTopic[] = "cats-eyes/status";
const char commandTopic[] = "cats-eyes/command";
const char settingsTopic[] = "cats-eyes/settings";
int count=0;
int gPingCount = 0;
char lMqttBuffer[MAX_MQTT_PACKET];

//----------------------------------------------------------------------------
//  Purpose:
//   Initialize the MQTT client and connect to the configured broker.
//
//  Notes:
//
//----------------------------------------------------------------------------
void mqttInit()
{
  mqttClient.setId("clientId");
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(getBroker());

  if (!mqttClient.connect(getBroker(), port)) {
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
    mqttClient.onMessage(onMqttMessage);
    mqttClient.subscribe(commandTopic);
  }

  mqttClient.beginMessage(healthTopic);
  mqttClient.print("{ \"cmd\": \"start\"");
  mqttClient.endMessage();

  mqttClient.beginMessage(healthTopic);
  mqttClient.print("{ \"cmd\": \"version\", \"data\": \"");
  mqttClient.print(APP_VERSION_MAJOR);
  mqttClient.print(".");
  mqttClient.print(APP_VERSION_MINOR);
  mqttClient.print("\"}");
  mqttClient.endMessage();


  sendPing();
}


//----------------------------------------------------------------------------
//  Purpose:
//   Poll the MQTT client and send periodic keepalive messages.
//
//  Notes:
//
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
//  Purpose:
//   Publish the current eye state change to the MQTT status topic.
//
//  Notes:
//
//----------------------------------------------------------------------------
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

  log_d("number=%d color=%s state=%s lastStateTime=%d timeFromOpen=%d",number,getColorName(color),getStateName(state),lastCount,sinceOpen);
}

//----------------------------------------------------------------------------
//  Purpose:
//   Send a health ping message to the MQTT broker.
//
//  Notes:
//
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
//  Purpose:
//   Handle an incoming MQTT message and dispatch the command payload.
//
//  Notes:
//
//----------------------------------------------------------------------------
void onMqttMessage(int messageSize) 
{
  // we received a message, print out the topic and contents
  String topic = mqttClient.messageTopic();
  log_d("Received:%s len:%d",topic.c_str(),messageSize);

  // use the Stream interface to print the contents
  uint16_t loc = 0;
  while (mqttClient.available()) 
  {
    lMqttBuffer[loc] = (char)mqttClient.read();
    loc++;
  }
  lMqttBuffer[loc] = 0x00;

  if(true == topic.equals(commandTopic))
  {
    log_i("Received:%s",lMqttBuffer);
    setSetting(lMqttBuffer);
  }
  else
  {
    log_w("Received messaage with topic:%s",topic.c_str());
  }
}

//----------------------------------------------------------------------------
//  Purpose:
//   Send an MQTT error payload describing the failed command.
//
//  Notes:
//
//----------------------------------------------------------------------------
void sendError(char* command, char *message)
{
  mqttClient.beginMessage(healthTopic);
  mqttClient.print("{ \"error\": \"");
  mqttClient.print(command);
  mqttClient.print("\", \"data\": \"");
  mqttClient.print(message);
  mqttClient.print("\"}");
  mqttClient.endMessage();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Send a string setting update over MQTT.
//
//  Notes:
//
//----------------------------------------------------------------------------
void sendSettings(char* setting, char *value)
{
  mqttClient.beginMessage(settingsTopic);
  mqttClient.print("{ \"setting\": \"");
  mqttClient.print(setting);
  mqttClient.print("\", \"value\": \"");
  mqttClient.print(value);
  mqttClient.print("\"}");
  mqttClient.endMessage();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Send a 16-bit setting update over MQTT.
//
//  Notes:
//
//----------------------------------------------------------------------------
void sendSettings(char* setting, uint16_t value)
{
  mqttClient.beginMessage(settingsTopic);
  mqttClient.print("{ \"setting\": \"");
  mqttClient.print(setting);
  mqttClient.print("\", \"value\": \"");
  mqttClient.print(value);
  mqttClient.print("\"}");
  mqttClient.endMessage();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Send a 32-bit setting update over MQTT.
//
//  Notes:
//
//----------------------------------------------------------------------------
void sendSettings(char* setting, uint32_t value)
{
  mqttClient.beginMessage(settingsTopic);
  mqttClient.print("{ \"setting\": \"");
  mqttClient.print(setting);
  mqttClient.print("\", \"value\": \"");
  mqttClient.print(value);
  mqttClient.print("\"}");
  mqttClient.endMessage();
}
