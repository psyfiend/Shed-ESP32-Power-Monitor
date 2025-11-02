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

PowerSubMode currentPowerSubMode = LIVE_POWER;

int lightsMenuSelection = 0;

// --- Light State Variables (Synced from Sensor Hub via MQTT) ---
bool lightIsOn = false;
bool lightManualOverride = false; // We can infer this from which timer is active
bool occupancyDetected = false;
unsigned long timerRemainingSeconds = 0;
unsigned long motionTimerDuration = MOTION_TIMER_DURATION;  // Default, will be updated by MQTT
unsigned long manualTimerDuration = MANUAL_TIMER_DURATION; // Default, will be updated by MQTT
unsigned long lightOnTime = 0; 
// Sensor Data Variables
float temperatureShed = 0.0;
float humidityShed = 0.0;
float pressureShed = 0.0;
float luxShed = 0.0;

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


// --- MQTT Update Handlers (for UI) ---
void handle_light_state_update(String message);
void handle_timer_remaining_update(String message);
void handle_motion_timer_state_update(String message);
void handle_manual_timer_state_update(String message);
void handle_occupancy_state_update(String message);

// --- Sensor State Update Handlers (for UI) ---
void handle_temperature_update(String message);
void handle_humidity_update(String message);
void handle_pressure_update(String message);
void handle_lux_update(String message);

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
  
  handle_input(); // Handle user input
  loop_power_monitor(); // Run core logic for this device

  // Inactivity timer to reset the view
  if (millis() - lastUserActivityTime > INACTIVITY_TIMEOUT) {
    currentMode = POWER_MODE_ALL;
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
    data.occupancyDetected = occupancyDetected;
    data.timerRemainingSeconds = timerRemainingSeconds;
    data.motionTimerDuration = motionTimerDuration;
    data.manualTimerDuration = manualTimerDuration;
    // Sensor Data
    data.temperature = temperatureShed;
    data.humidity = humidityShed;
    data.barometricPressure = pressureShed;
    data.lux = luxShed;
    // Menu/UI State Data
    data.lightsMenuSelection = lightsMenuSelection;
    data.tempMotionTimerDuration = tempMotionTimerDuration;
    data.tempManualTimerDuration = tempManualTimerDuration;
        
    // Call the main display handler
    update_display(currentMode, currentPowerSubMode, data);
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
  
  // --- UPDATED: New simplified state logic ---
  switch (currentMode) {
    case POWER_MODE_ALL:
    case POWER_MODE_CH1:
    case POWER_MODE_CH2:
    case POWER_MODE_CH3:
    case SENSORS_MODE: // New screen included in this logic
      // Handle knob turning (cycles through main screens)
      if (encoderChange != 0) {
        int modeIndex = (int)currentMode;
        if (encoderChange > 0) modeIndex++;
        else modeIndex--;
        
        // Cycle from SENSORS_MODE back to POWER_MODE_ALL
        if (modeIndex > (int)SENSORS_MODE) modeIndex = (int)POWER_MODE_ALL;
        // Cycle from POWER_MODE_ALL back to SENSORS_MODE
        if (modeIndex < (int)POWER_MODE_ALL) modeIndex = (int)SENSORS_MODE; 
        
        currentMode = (DisplayMode)modeIndex;
        currentPowerSubMode = LIVE_POWER; // Reset submode when changing main screen
      }
      
      // Handle button presses
      if (buttonPressed) {
        if (currentMode == POWER_MODE_ALL) {
          // Button press on home screen opens the lights menu
          currentMode = LIGHTS_MENU;
          lightsMenuSelection = 0;
        } else if (currentMode == POWER_MODE_CH1 || currentMode == POWER_MODE_CH2 || currentMode == POWER_MODE_CH3) {
          // Button press on channel screens toggles sub-screen
          currentPowerSubMode = (currentPowerSubMode == LIVE_POWER) ? POWER_SUBSCREEN : LIVE_POWER;
        } else if (currentMode == SENSORS_MODE) {
          // Button press on sensor screen returns to home
          currentMode = POWER_MODE_ALL;
        }
      }
      break;

    // Handle menu navigation logic
    case LIGHTS_MENU:
      if (encoderChange != 0) {
        lightsMenuSelection += (encoderChange > 0) ? 1 : -1;
        if (lightsMenuSelection < 0) lightsMenuSelection = LIGHTS_MENU_ITEM_COUNT - 1;
        if (lightsMenuSelection >= LIGHTS_MENU_ITEM_COUNT) lightsMenuSelection = 0;
      }
      if (buttonPressed) {
        switch (lightsMenuSelection) {
            case 0:   // Turn light On/Off
              if (lightIsOn) {
                client.publish(MQTT_TOPIC_LIGHT_COMMAND, "OFF");
              } else {
                client.publish(MQTT_TOPIC_LIGHT_COMMAND, "ON");
                lightManualOverride = true; // Set manual override when turned on via UI
              }
              break; 
            case 1:
              tempMotionTimerDuration = motionTimerDuration;
              currentMode = EDIT_MOTION_TIMER; // Change main mode
              break;
            case 2:
              tempManualTimerDuration = manualTimerDuration;
              currentMode = EDIT_MANUAL_TIMER; // Change main mode
              break;
            case 3:
              currentMode = POWER_MODE_ALL; // Back to home screen
              break;
        }
      }
      break;

    case EDIT_MOTION_TIMER:
      if (encoderChange != 0) {
        if (encoderChange < 0) {
          if (tempMotionTimerDuration > 30000) tempMotionTimerDuration -= 30000;
          else tempMotionTimerDuration = 10000;
        } else {
          if (tempMotionTimerDuration < 3600000) tempMotionTimerDuration += 30000;
          else tempMotionTimerDuration = 3600000;
        }
      }
      if (buttonPressed) {
          client.publish(MQTT_TOPIC_MOTION_TIMER_COMMAND, String(tempMotionTimerDuration / 1000).c_str(), true);
          currentMode = LIGHTS_MENU; // Go back to menu
      }
      break;

    case EDIT_MANUAL_TIMER:
      if (encoderChange != 0) {
        if (encoderChange < 0) {
          if (tempManualTimerDuration > 30000) tempManualTimerDuration -= 30000;
          else tempManualTimerDuration = 10000;
        } else {
          if (tempManualTimerDuration < 3600000) tempManualTimerDuration += 30000;
          else tempManualTimerDuration = 3600000;
        }
      }
      if (buttonPressed) {
          client.publish(MQTT_TOPIC_MANUAL_TIMER_COMMAND, String(tempManualTimerDuration / 1000).c_str(), true);
          currentMode = LIGHTS_MENU; // Go back to menu
      }
      break;
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
    if (!lightIsOn) {
        lightManualOverride = false; // Clear manual override when light is turned off
    }

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

void handle_occupancy_state_update(String message) {
    message.toUpperCase();
    bool occupancyState = (message == "ON");
    occupancyDetected = occupancyState;

    Serial.print("UI Updated: Occupancy state is now ");
    Serial.println(message);
}

void handle_temperature_update(String message) {
    temperatureShed = message.toFloat();
}

void handle_humidity_update(String message) {
    humidityShed = message.toFloat();
}

void handle_pressure_update(String message) {
    pressureShed = message.toFloat();
}

void handle_lux_update(String message) {
    luxShed = message.toFloat();
}