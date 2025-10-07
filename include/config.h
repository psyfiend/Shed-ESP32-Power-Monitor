#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

static const int DEVICE_DISCOVERY_PAYLOAD_SIZE = 8192; // Size of the JSON payload for MQTT Discovery

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

// --- MQTT Topics ---
// extern const char* MQTT_BASE_TOPIC;
extern const char* MQTT_TOPIC_AVAILABILITY;
// extern const char* MQTT_TOPIC_MOTION_STATE;    // For the physical PIR sensor
// extern const char* MQTT_TOPIC_OCCUPANCY_STATE; // For the software timer
extern const char* MQTT_TOPIC_LIGHT_STATE;      // For the physical light state
extern const char* MQTT_TOPIC_LIGHT_COMMAND;    // For commands to control the light
// extern const char* MQTT_TOPIC_LUX_STATE;       // For ambient light sensor state

// --- Base Topics for MQTT Discovery ---
// extern const char* MQTT_TOPIC_LIGHT_BASE;       // shed/monitor/light
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE;  // shed/monitor/light/motion_timer"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE;  // shed/monitor/light/manual_timer"
extern const char* MQTT_TOPIC_POWER_CH1_STATE;     // shed/monitor/power/panel
extern const char* MQTT_TOPIC_POWER_CH2_STATE;     // shed/monitor/power/battery
extern const char* MQTT_TOPIC_POWER_CH3_STATE;     // shed/monitor/power/load

// --- Topics for functions to address
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE; // shed/monitor/light/motion_timer/state"
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_SET;   // shed/monitor/light/motion_timer/set"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE; // shed/monitor/light/manual_timer/state"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET;   // shed/monitor/light/manual_timer/set"

// --- MQTT Payloads ---
extern const char* MQTT_PAYLOAD_ONLINE;
extern const char* MQTT_PAYLOAD_OFFLINE;

#endif // CONFIG_H

