#include "config.h"
#include <Arduino.h> // For LED_BUILTIN





// ESP32 DevKitC
// I2C
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;

// --- Rotary Encoder
const int ENCODER_CLK_PIN = 25;
const int ENCODER_DT_PIN = 26;
const int ENCODER_SW_PIN = 27;

// INA226 Alert Pins
const int INA_ALERT_PIN_CH1 = 35; // Solar Panel
const int INA_ALERT_PIN_CH2 = 34; // Battery
const int INA_ALERT_PIN_CH3 = 32; // Load

// SPI Display
const int SPI_CS_PIN = 5;
const int SPI_SCK_PIN = 18;
const int SPI_MOSI_PIN = 23;
const int SPI_DC_PIN = 19;
const int SPI_RST_PIN = 4;
const int SPI_BLK_PIN = 16; // Optional, can be set to -1 if not used

// OLED display settings
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;
const int OLED_RESET = -1;
const int OLED_I2C_ADDRESS = 0x3C;

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "M&M Motors";
const char* WIFI_PASSWORD = "seamosss";

// --- MQTT Broker Settings ---
const char* MQTT_SERVER = "192.168.0.70";
const char* MQTT_USER = "mqtt_user";
const char* MQTT_PASSWORD = "mqtt_user3700";
const char* DEVICE_ID = "shed_power_monitor"; // Changed from shed_solar_monitor for clarity


// --- Power Monitor ---
const uint8_t INA226_CH1_ADDRESS = 0x40; // Solar Panel
const uint8_t INA226_CH2_ADDRESS = 0x41; // Battery 
const uint8_t INA226_CH3_ADDRESS = 0x44; // Load 
const float INA226_CH1_SHUNT = 0.01;    // Shunt resistor (10 milliohms)
const float INA226_CH2_SHUNT = 0.01;
const float INA226_CH3_SHUNT = 0.01;

// --- Application Logic Constants ---
unsigned long MOTION_TIMER_DURATION = 10000;      // 10 seconds
unsigned long MANUAL_TIMER_DURATION = 300000;     // 5 minutes (300,000 ms)
const int NUM_MODES = 5;
const unsigned long INACTIVITY_TIMEOUT = 30000;
const int DISPLAY_UPDATE_INTERVAL = 100;

// --- Grand Unified MQTT Topics ---
// This new structure follows the home/[location]/[domain]/[object_id]/[message_type] pattern.

// --- Base Topics for Discovery Payloads ---
const char* MQTT_BASE_TOPIC_LIGHT = "home/shed/light/main";
const char* MQTT_BASE_TOPIC_MOTION_TIMER = "home/shed/number/motion_timer";
const char* MQTT_BASE_TOPIC_MANUAL_TIMER = "home/shed/number/manual_timer";

// --- Device & Sensor Availability Topics ---
const char* MQTT_TOPIC_DEVICE_AVAILABILITY = "devices/shed_power_monitor/status";
const char* MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY = "devices/shed_power_monitor/panel_sensor_status";
const char* MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY = "devices/shed_power_monitor/battery_sensor_status";
const char* MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY = "devices/shed_power_monitor/load_sensor_status";

// --- Light Control Topics (interacting with Sensor Hub) ---
const char* MQTT_TOPIC_LIGHT_STATE = "home/shed/light/main/state";
const char* MQTT_TOPIC_LIGHT_COMMAND = "home/shed/light/main/command";

// --- Timer Control Topics (interacting with Sensor Hub) ---
const char* MQTT_TOPIC_MOTION_TIMER_STATE = "home/shed/number/motion_timer/state";
const char* MQTT_TOPIC_MOTION_TIMER_COMMAND = "home/shed/number/motion_timer/command";
const char* MQTT_TOPIC_MANUAL_TIMER_STATE = "home/shed/number/manual_timer/state";
const char* MQTT_TOPIC_MANUAL_TIMER_COMMAND = "home/shed/number/manual_timer/command";

// --- Power Monitor Sensor Topics (Published by this device) ---
// --- Channel 1: Solar Panel ---
const char* MQTT_TOPIC_SOLAR_PANEL_VOLTAGE_STATE = "home/shed/sensor/solar_panel_voltage/state";
const char* MQTT_TOPIC_SOLAR_PANEL_CURRENT_STATE = "home/shed/sensor/solar_panel_current/state";
const char* MQTT_TOPIC_SOLAR_PANEL_POWER_STATE = "home/shed/sensor/solar_panel_power/state";
const char* MQTT_TOPIC_SOLAR_PANEL_ENERGY_STATE = "home/shed/sensor/solar_panel_energy/state";

// --- Channel 2: Battery ---
const char* MQTT_TOPIC_BATTERY_VOLTAGE_STATE = "home/shed/sensor/solar_battery_voltage/state";
const char* MQTT_TOPIC_BATTERY_CURRENT_STATE = "home/shed/sensor/solar_battery_current/state";
const char* MQTT_TOPIC_BATTERY_POWER_STATE = "home/shed/sensor/solar_battery_power/state";
const char* MQTT_TOPIC_BATTERY_ENERGY_CHARGED_STATE = "home/shed/sensor/solar_battery_energy_charged/state";
const char* MQTT_TOPIC_BATTERY_ENERGY_DISCHARGED_STATE = "home/shed/sensor/solar_battery_energy_discharged/state";

// --- Channel 3: Load ---
const char* MQTT_TOPIC_LOAD_VOLTAGE_STATE = "home/shed/sensor/solar_load_voltage/state";
const char* MQTT_TOPIC_LOAD_CURRENT_STATE = "home/shed/sensor/solar_load_current/state";
const char* MQTT_TOPIC_LOAD_POWER_STATE = "home/shed/sensor/solar_load_power/state";
const char* MQTT_TOPIC_LOAD_ENERGY_STATE = "home/shed/sensor/solar_load_energy/state";


// --- MQTT Payloads ---
const char* MQTT_PAYLOAD_ONLINE = "online";
const char* MQTT_PAYLOAD_OFFLINE = "offline";


