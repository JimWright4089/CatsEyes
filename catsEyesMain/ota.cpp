//----------------------------------------------------------------------------
//
//  Workfile: ota.cpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     OTA update and Wi-Fi connection implementation.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
#include "ota.hpp"
#include "settings.hpp"

uint32_t lLastOtaTime = 0;
char lName[SIZE_OF_STRING];

//----------------------------------------------------------------------------
//  Purpose:
//   Connect the device to the configured Wi-Fi network.
//
//  Notes:
//
//----------------------------------------------------------------------------
void wifiInit(void)
{
  WiFi.mode(WIFI_STA);

  WiFi.begin((const char*)getSsid(), (const char*)getPassword());
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    log_e("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//----------------------------------------------------------------------------
//  Purpose:
//   Initialize the Arduino OTA update handler and callbacks.
//
//  Notes:
//
//----------------------------------------------------------------------------
void otaInit(void)
{

  snprintf(lName,SIZE_OF_STRING,"cats-eye-%d",getID());
  ArduinoOTA.setHostname(lName);
  ArduinoOTA.setPort(3232);
  ArduinoOTA.setPassword("cats=5-eyes");

  // Or set password with pre-hashed value (SHA256 hash of "admin")
  // SHA256(admin) = 8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918
  // ArduinoOTA.setPasswordHash("8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      if (millis() - lLastOtaTime > 500) {
        Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
        lLastOtaTime = millis();
      }
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();
}

//----------------------------------------------------------------------------
//  Purpose:
//   Process OTA updates while the application is running.
//
//  Notes:
//
//----------------------------------------------------------------------------
void otaRun(void) 
{
  ArduinoOTA.handle();
}
