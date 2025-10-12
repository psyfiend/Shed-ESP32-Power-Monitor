#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "discovery.h"
#include "connections.h"
#include "config.h"
#include "power_monitor.h"

extern PubSubClient client;

// main.cpp functions to handle UI updates via MQTT
extern void handle_light_state_update(String message);
extern void handle_motion_timer_state_update(String message);
extern void handle_manual_timer_state_update(String message);
extern void handle_timer_remaining_update(String message);
extern bool is_sensor_online(int channel);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.setHostname(DEVICE_ID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload to a printable string
  payload[length] = '\0'; // Add a null terminator
  String message = (char*)payload;

  // Add a filter to prevent spamming the serial monitor ---
  if (String(topic) != MQTT_TOPIC_TIMER_REMAINING_STATE) {
    Serial.println("--- MQTT Message Received ---");
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Payload: ");
    Serial.println(message);
    Serial.println("-----------------------------");
  }
  
  // ---- Route messages based on topic ----
  if (String(topic) == MQTT_TOPIC_LIGHT_STATE) {
    handle_light_state_update(message);
  } else if (String(topic) == MQTT_TOPIC_MOTION_TIMER_STATE) {
    handle_motion_timer_state_update(message);
  } else if (String(topic) == MQTT_TOPIC_MANUAL_TIMER_STATE) {
    handle_manual_timer_state_update(message);
  } else if (String(topic) == MQTT_TOPIC_TIMER_REMAINING_STATE) {
    handle_timer_remaining_update(message);
  }
}


void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32-Solar-Monitor";

  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_DEVICE_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    Serial.println("connected!");
    Serial.println("------------------------------");
    
    // Publish device and sensor availability
    client.publish(MQTT_TOPIC_DEVICE_AVAILABILITY, MQTT_PAYLOAD_ONLINE, true);
    client.publish(MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY, is_sensor_online(1) ? MQTT_PAYLOAD_ONLINE : MQTT_PAYLOAD_OFFLINE, true);
    client.publish(MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY, is_sensor_online(2) ? MQTT_PAYLOAD_ONLINE : MQTT_PAYLOAD_OFFLINE, true);
    client.publish(MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY, is_sensor_online(3) ? MQTT_PAYLOAD_ONLINE : MQTT_PAYLOAD_OFFLINE, true);
    Serial.println("Published device and sensor availability.");
    
    // Publish the default timers (in seconds)
//    Serial.println("------------------------------");
    
//    String motion_payload = String(MOTION_TIMER_DURATION / 1000);
//    client.publish(MQTT_TOPIC_MOTION_TIMER_STATE, motion_payload.c_str(), true);

//    String manual_payload = String(MANUAL_TIMER_DURATION / 1000);
//    client.publish(MQTT_TOPIC_MANUAL_TIMER_STATE, manual_payload.c_str(), true);
    
//    Serial.println("Published initial timer states.");

    // Subscribe to specific light topics, apply retained values if broker is online
    Serial.println("------------------------------");
    client.subscribe(MQTT_TOPIC_LIGHT_STATE);
    client.subscribe(MQTT_TOPIC_MOTION_TIMER_STATE);
    client.subscribe(MQTT_TOPIC_MANUAL_TIMER_STATE);
    client.subscribe(MQTT_TOPIC_TIMER_REMAINING_STATE);
    Serial.println("Subscribed to command topics.");

    // Publish the discovery message
    mqtt_discovery();

  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

