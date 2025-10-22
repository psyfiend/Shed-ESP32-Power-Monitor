#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

static const int DEVICE_DISCOVERY_PAYLOAD_SIZE = 7168; // Size of the JSON payload for MQTT Discovery

// ESP32 DevKitC
// I2C
extern const int I2C_SDA_PIN;
extern const int I2C_SCL_PIN;

// Rotary Encoder
extern const int ENCODER_CLK_PIN;
extern const int ENCODER_DT_PIN;
extern const int ENCODER_SW_PIN;

// INA226 Alert Pins
extern const int INA_ALERT_PIN_CH1;
extern const int INA_ALERT_PIN_CH2;
extern const int INA_ALERT_PIN_CH3;

// SPI Display
extern const int SPI_CS_PIN;
extern const int SPI_SCK_PIN;
extern const int SPI_MOSI_PIN;
extern const int SPI_DC_PIN;
extern const int SPI_RST_PIN;
extern const int SPI_BLK_PIN;

// OLED display settings
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int OLED_RESET;
extern const int OLED_I2C_ADDRESS;

// --- Wi-Fi Credentials ---
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// --- MQTT Broker Settings ---
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* DEVICE_ID;

// --- Power Monitor ---
extern const uint8_t INA226_CH1_ADDRESS;
extern const uint8_t INA226_CH2_ADDRESS;
extern const uint8_t INA226_CH3_ADDRESS;
extern const float INA226_CH1_SHUNT;
extern const float INA226_CH2_SHUNT;
extern const float INA226_CH3_SHUNT;

// --- Application Logic Constants ---
extern unsigned long MOTION_TIMER_DURATION;
extern unsigned long MANUAL_TIMER_DURATION;
extern const int NUM_MODES;
extern const unsigned long INACTIVITY_TIMEOUT;
extern const int DISPLAY_UPDATE_INTERVAL;

// --- Grand Unified MQTT Topics ---

// --- Base Topics for Discovery Payloads ---
extern const char* MQTT_BASE_TOPIC_LIGHT;
extern const char* MQTT_BASE_TOPIC_MOTION_TIMER;
extern const char* MQTT_BASE_TOPIC_MANUAL_TIMER;

// --- Device & Sensor Availability Topics ---
extern const char* MQTT_TOPIC_DEVICE_AVAILABILITY;
extern const char* MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY;
extern const char* MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;
extern const char* MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY;

// --- Light Control Topics (interacting with Sensor Hub) ---
extern const char* MQTT_TOPIC_LIGHT_STATE;
extern const char* MQTT_TOPIC_LIGHT_COMMAND;
extern const char* MQTT_TOPIC_OCCUPANCY_STATE;  //home/shed/binary_sensor/occupancy/state

// --- Timer Control Topics (interacting with Sensor Hub) ---
extern const char* MQTT_TOPIC_MOTION_TIMER_STATE;
extern const char* MQTT_TOPIC_MOTION_TIMER_COMMAND;
extern const char* MQTT_TOPIC_MANUAL_TIMER_STATE;
extern const char* MQTT_TOPIC_MANUAL_TIMER_COMMAND;
extern const char* MQTT_TOPIC_TIMER_REMAINING_STATE;

// --- Power Monitor Sensor Topics (Published by this device) ---
// --- Channel 1: Solar Panel ---
extern const char* MQTT_TOPIC_SOLAR_PANEL_VOLTAGE_STATE;
extern const char* MQTT_TOPIC_SOLAR_PANEL_CURRENT_STATE;
extern const char* MQTT_TOPIC_SOLAR_PANEL_POWER_STATE;
extern const char* MQTT_TOPIC_SOLAR_PANEL_ENERGY_STATE;

// --- Channel 2: Battery ---
extern const char* MQTT_TOPIC_BATTERY_VOLTAGE_STATE;
extern const char* MQTT_TOPIC_BATTERY_CURRENT_STATE;
extern const char* MQTT_TOPIC_BATTERY_POWER_STATE;
extern const char* MQTT_TOPIC_BATTERY_ENERGY_CHARGED_STATE;
extern const char* MQTT_TOPIC_BATTERY_ENERGY_DISCHARGED_STATE;

// --- Channel 3: Load ---
extern const char* MQTT_TOPIC_LOAD_VOLTAGE_STATE;
extern const char* MQTT_TOPIC_LOAD_CURRENT_STATE;
extern const char* MQTT_TOPIC_LOAD_POWER_STATE;
extern const char* MQTT_TOPIC_LOAD_ENERGY_STATE;

// --- MQTT Payloads ---
extern const char* MQTT_PAYLOAD_ONLINE;
extern const char* MQTT_PAYLOAD_OFFLINE;

#endif // CONFIG_H



