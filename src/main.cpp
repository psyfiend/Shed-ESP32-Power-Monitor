#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Include our modularized files
#include "config.h"
#include "connections.h"
#include "encoder.h"
#include "power_monitor.h"
#include "display_manager.h"
#include "utils.h"
#include "ota_manager.h"

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- State Tracking Variables ---
DisplayMode currentMode = POWER_MODE_ALL;
LightsSubMode currentLightsSubMode = LIVE_STATUS;
PowerSubMode currentPowerSubMode = LIVE_POWER;

int lightsMenuSelection = 0;

// --- Light State Variables (Synced from Sensor Hub via MQTT) ---
bool lightIsOn = false;
bool lightManualOverride = false; // We can infer this from which timer is active
unsigned long timerRemainingSeconds = 0;
unsigned long motionTimerDuration = MOTION_TIMER_DURATION;  // Default, will be updated by MQTT
unsigned long manualTimerDuration = MANUAL_TIMER_DURATION; // Default, will be updated by MQTT
unsigned long lightOnTime = 0; 

// --- Temporary variables for editing timers ---
unsigned long tempMotionTimerDuration;
unsigned long tempManualTimerDuration;

// --- Non-Blocking Timers ---
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastMqttReconnectAttempt = 0;
unsigned long lastUserActivityTime = 0;
int lastEncoderValue = 0;

// --- Forward Declarations ---
void handle_input();
void handle_lights_input(int encoderChange, bool buttonPressed);
void handle_power_input(int encoderChange, bool buttonPressed);

// --- MQTT Update Handlers (for UI) ---
void handle_light_state_update(String message);
void handle_timer_remaining_update(String message);
void handle_motion_timer_state_update(String message);
void handle_manual_timer_state_update(String message);


void setup() {
  Serial.begin(115200);

  setup_display();
  setup_encoder();
  setup_wifi();
  setup_power_monitor();
  
  // Configure MQTT client
  client.setServer(MQTT_SERVER, 1883);
  client.setBufferSize(DEVICE_DISCOVERY_PAYLOAD_SIZE);
  client.setCallback(mqtt_callback);
  
  // Initialize OTA
  setup_ota();

  lastUserActivityTime = millis();
  lastEncoderValue = get_encoder_value();
}

void loop() {
  loop_ota();

  if (!client.connected()) {
    long now = millis();
    if (now - lastMqttReconnectAttempt > 5000) {
      lastMqttReconnectAttempt = now;
      reconnect();
    }
  } else {
    client.loop();
  }
  
  //  loop_encoder(); // Interpret knob/button input  --- No longer needed with stable ISR for button and knob
  handle_input(); // Handle user input
  loop_power_monitor(); // Run core logic for this device

  // Inactivity timer to reset the view
  if (millis() - lastUserActivityTime > INACTIVITY_TIMEOUT) {
    currentMode = POWER_MODE_ALL;
    currentLightsSubMode = LIVE_STATUS;
    currentPowerSubMode = LIVE_POWER;
  }

  // Update the display on a non-blocking timer
  if (millis() - lastDisplayUpdateTime > DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdateTime = millis();
    
    // Package up the current state into a data structure
    DisplayData data;
    // Power Data
    for(int i=0; i<3; i++) {
      data.busVoltage[i] = get_bus_voltage(i+1);
      data.current[i] = get_current(i+1);
      data.power[i] = get_power(i+1);
    }
    // Light Status Data
    data.lightIsOn = lightIsOn;
    data.lightManualOverride = lightManualOverride; // This needs to be inferred or sent
    data.timerRemainingSeconds = timerRemainingSeconds;
    data.motionTimerDuration = motionTimerDuration;
    data.manualTimerDuration = manualTimerDuration;
    // Menu/UI State Data
    data.lightsMenuSelection = lightsMenuSelection;
    data.tempMotionTimerDuration = tempMotionTimerDuration;
    data.tempManualTimerDuration = tempManualTimerDuration;
        
    // Call the main display handler
    update_display(currentMode, currentLightsSubMode, currentPowerSubMode, data);
  }
}

// --- Central Input Dispatcher ---
void handle_input() {
  int currentEncoderValue = get_encoder_value();
  int encoderChange = 0;
  if (currentEncoderValue != lastEncoderValue) {
    encoderChange = currentEncoderValue - lastEncoderValue;
    lastEncoderValue = currentEncoderValue;
    lastUserActivityTime = millis();
  }
  
  bool buttonPressed = button_was_clicked();
  if (buttonPressed) {
    lastUserActivityTime = millis();
  }

  if (encoderChange == 0 && !buttonPressed) return; // No input, do nothing
  
  // Route the input to the correct specialized handler
  switch (currentMode) {
    case LIGHTS_MODE:
      handle_lights_input(encoderChange, buttonPressed);
      break;
    case POWER_MODE_ALL:
    case POWER_MODE_CH1:
    case POWER_MODE_CH2:
    case POWER_MODE_CH3:
      handle_power_input(encoderChange, buttonPressed);
      break;
  }
}


// --- Specialized Input Handlers ---

void handle_lights_input(int encoderChange, bool buttonPressed) {
  if (encoderChange != 0) {
    switch (currentLightsSubMode) {
      case LIGHTS_MENU:
        lightsMenuSelection += (encoderChange > 0) ? 1 : -1;
        if (lightsMenuSelection < 0) lightsMenuSelection = LIGHTS_MENU_ITEM_COUNT - 1;
        if (lightsMenuSelection >= LIGHTS_MENU_ITEM_COUNT) lightsMenuSelection = 0;
        break;
      case EDIT_MOTION_TIMER:
        if (encoderChange < 0) {
          if (tempMotionTimerDuration > 30000) tempMotionTimerDuration -= 30000;
          else tempMotionTimerDuration = 10000;
        } else {
          if (tempMotionTimerDuration < 3600000) tempMotionTimerDuration += 30000;
          else tempMotionTimerDuration = 3600000;
        }
        break;
      case EDIT_MANUAL_TIMER:
        if (encoderChange < 0) {
          if (tempManualTimerDuration > 30000) tempManualTimerDuration -= 30000;
          else tempManualTimerDuration = 10000;
        } else {
          if (tempManualTimerDuration < 3600000) tempManualTimerDuration += 30000;
          else tempManualTimerDuration = 3600000;
        }
        break;
      case LIVE_STATUS:
        {
        // If we are on the live screen, knob turn changes main mode
          int modeIndex = (int)currentMode;
          if (encoderChange > 0) modeIndex++;
          else modeIndex--;
          if (modeIndex < 0) modeIndex = NUM_MODES - 1;
          if (modeIndex >= NUM_MODES) modeIndex = 0;
          currentMode = (DisplayMode)modeIndex;
        }
        break;
    }
  }

  // Handle button presses
  if (buttonPressed) {
    switch (currentLightsSubMode) {
      case LIVE_STATUS:
        currentLightsSubMode = LIGHTS_MENU;
        lightsMenuSelection = 0;
        break;
      case LIGHTS_MENU:
        switch (lightsMenuSelection) {
            case 0:   // Turn light On/Off
              if (lightIsOn) {
                client.publish(MQTT_TOPIC_LIGHT_COMMAND, "OFF");
              } else {
                client.publish(MQTT_TOPIC_LIGHT_COMMAND, "ON");
              break;
              }
            case 1:
              tempMotionTimerDuration = motionTimerDuration;
              currentLightsSubMode = EDIT_MOTION_TIMER;
              break;
            case 2:
              tempManualTimerDuration = manualTimerDuration;
              currentLightsSubMode = EDIT_MANUAL_TIMER;
              break;
            case 3:
              currentLightsSubMode = LIVE_STATUS;
              break;
        }
        break;
      case EDIT_MOTION_TIMER:
          client.publish(MQTT_TOPIC_MOTION_TIMER_COMMAND, String(tempMotionTimerDuration / 1000).c_str(), true);
          currentLightsSubMode = LIGHTS_MENU;
          break;
      case EDIT_MANUAL_TIMER:
          client.publish(MQTT_TOPIC_MANUAL_TIMER_COMMAND, String(tempManualTimerDuration / 1000).c_str(), true);
          currentLightsSubMode = LIGHTS_MENU;
          break;
      }
    }
}

// --- Specialized Input Handler for all Power Modes ---
void handle_power_input(int encoderChange, bool buttonPressed) {
  // Handle knob turning
  if (encoderChange != 0) {
    // Only allow changing main mode from a top-level screen
    if (currentPowerSubMode == LIVE_POWER) {
      int modeIndex = (int)currentMode;
      if (encoderChange > 0) modeIndex++;
      else modeIndex--;
      if (modeIndex < 0) modeIndex = NUM_MODES - 1;
      if (modeIndex >= NUM_MODES) modeIndex = 0;
      currentMode = (DisplayMode)modeIndex;
    }
  }

  // Handle button presses
  if (buttonPressed) { // <---- UPDATED (Complete logic restored)
    if (currentMode != POWER_MODE_ALL) {
      // Toggle subscreen for channel-specific views
      currentPowerSubMode = (currentPowerSubMode == LIVE_POWER) ? POWER_SUBSCREEN : LIVE_POWER;
    }
  }
}


// --- MQTT Update Handlers ---
void handle_light_state_update(String message) {
    message.toUpperCase();
    bool newLightState = (message == "ON");

    if (newLightState && !lightIsOn) { // <---- UPDATED (If state is changing to ON)
        lightOnTime = millis();       // <---- ADDED (Record the timestamp)
    }
    lightIsOn = newLightState;

    Serial.print("UI Updated: Light state is now ");
    Serial.println(message);
}

void handle_motion_timer_state_update(String message) {
    motionTimerDuration = message.toInt() * 1000;
}

void handle_manual_timer_state_update(String message) {
    manualTimerDuration = message.toInt() * 1000;
}

void handle_timer_remaining_update(String message) {
    timerRemainingSeconds = message.toInt();
}