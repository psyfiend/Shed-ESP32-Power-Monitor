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
const char* DEVICE_ID = "shed_power_monitor";

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

// --- MQTT Topics ---
const char* MQTT_BASE_TOPIC = "shed/monitor";
const char* MQTT_TOPIC_AVAILABILITY = "shed/monitor/availability";
const char* MQTT_TOPIC_MOTION_STATE = "shed/monitor/motion/state";       // Physical PIR sensor state
const char* MQTT_TOPIC_OCCUPANCY_STATE = "shed/monitor/occupancy/state"; // Software-based occupancy state (based on timer)
const char* MQTT_TOPIC_LIGHT_STATE = "shed/monitor/light/state";         // Physical light state
const char* MQTT_TOPIC_LIGHT_COMMAND = "shed/monitor/light/switch";      // Command to control the light state
const char* MQTT_TOPIC_LUX_STATE = "shed/monitor/lux/state";             // Ambient light sensor state

// --- Base Topics for MQTT Discovery ---
const char* MQTT_TOPIC_LIGHT_BASE = "shed/monitor/light";
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE = "shed/monitor/light/motion_timer";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE = "shed/monitor/light/manual_timer";
const char* MQTT_TOPIC_POWER_CH1_STATE = "shed/monitor/power/ch1";
const char* MQTT_TOPIC_POWER_CH2_STATE = "shed/monitor/power/ch2";
const char* MQTT_TOPIC_POWER_CH2_ENERGY_IN_STATE = "shed/monitor/power/ch2/energy_in";
const char* MQTT_TOPIC_POWER_CH2_ENERGY_OUT_STATE = "shed/monitor/power/ch2/energy_out";
const char* MQTT_TOPIC_POWER_CH3_STATE = "shed/monitor/power/ch3";

// --- Topics for functions to address
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE = "shed/monitor/light/motion_timer/state";
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_SET = "shed/monitor/light/motion_timer/set";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE = "shed/monitor/light/manual_timer/state";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET = "shed/monitor/light/manual_timer/set";

// --- MQTT Payloads ---
const char* MQTT_PAYLOAD_ONLINE = "online";
const char* MQTT_PAYLOAD_OFFLINE = "offline";
