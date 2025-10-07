#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "discovery.h"
#include "connections.h"
#include "config.h"

extern PubSubClient client;

// main.cpp defines these functions to handle the specific MQTT commands
extern void handle_light_state_update(String message);
extern void handle_motion_timer_command(String message);
extern void handle_manual_timer_command(String message);

// These are defined in main.cpp, but our callback needs to control them
extern bool lightManualOverride;
extern unsigned long lastMotionTime;
extern unsigned long MOTION_TIMER_DURATION;
extern unsigned long MANUAL_TIMER_DURATION;

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

  Serial.println("--- MQTT Message Received ---");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);
  Serial.println("-----------------------------");

  // ---- Route messages based on topic ----
    if (String(topic) == MQTT_TOPIC_LIGHT_STATE) {
    handle_light_state_update(message);

    } else if (String(topic) == MQTT_TOPIC_LIGHT_MOTION_TIMER_SET) {
    handle_motion_timer_command(message);

    } else if (String(topic) == MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET) {
    handle_manual_timer_command(message);
  }
}


void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32-Solar-Monitor";

  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    Serial.println("connected!");
    Serial.println("------------------------------");
    
    // Birth message and initial states
    client.publish(MQTT_TOPIC_AVAILABILITY, MQTT_PAYLOAD_ONLINE, true);
    
    // Publish the default timers (in seconds)
    Serial.println("------------------------------");
    
    String motion_payload = String(MOTION_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE, motion_payload.c_str(), true);

    String manual_payload = String(MANUAL_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE, manual_payload.c_str(), true);
    
    Serial.println("Published initial timer states.");

    // Subscribe to specific light topics, apply retained values if broker is online
    Serial.println("------------------------------");
    client.subscribe(MQTT_TOPIC_LIGHT_STATE);
    client.subscribe(MQTT_TOPIC_LIGHT_MOTION_TIMER_SET);
    client.subscribe(MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET);
    Serial.print("Subscribed to command topics.");

    // Publish the discovery message
    mqtt_discovery();

  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

