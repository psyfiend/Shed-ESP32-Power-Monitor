#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>      // Needed for WiFi.localIP()
#include "ota_manager.h"
#include "config.h" // Needed for the DEVICE_ID

void setup_ota() {
  // Set the hostname for the device. This is how it will appear on your network.
  // The .local suffix is automatically handled by the mDNS protocol.
  ArduinoOTA.setHostname(DEVICE_ID);

  // You can also set a password for updates for extra security
  // ArduinoOTA.setPassword("your_password");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  Serial.println("OTA Manager Initialized.");
  Serial.print("Ready to receive updates at hostname: ");
  Serial.print(DEVICE_ID);
  Serial.println(".local");
}

void loop_ota() {
  // This must be called on every loop to listen for incoming update requests.
  ArduinoOTA.handle();
}