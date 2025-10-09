#include <Arduino.h>
#include <Wire.h> 
#include <INA226.h>
#include <Adafruit_INA219.h>
#include <PubSubClient.h>
#include "connections.h"
#include "power_monitor.h"
#include "config.h"

// Pointers are initialized to nullptr to indicate they are not yet assigned.
// --- MODIFICATION: ina_ch1 is now an INA219, ch2 and ch3 are still INA226 ---
Adafruit_INA219 *ina_ch1 = nullptr; // <-- CHANGED
// INA226 *ina_ch1 = nullptr;
INA226 *ina_ch2 = nullptr;
INA226 *ina_ch3 = nullptr;

// --- Add a new global array to track sensor status ---
bool sensor_online[3] = {false, false, false};

// Variables to hold the latest sensor readings for all 3 channels
float busVoltage[3] = {0.0, 0.0, 0.0};
float current_ma[3] = {0.0, 0.0, 0.0};
float power_mw[3] = {0.0, 0.0, 0.0};
float totalEnergyWh[3] = {0.0, 0.0, 0.0}; // Variables to hold cumulative energy in Watt-hours for all 3 channels
float batteryEnergyChargeWh = 0.0;
float batteryEnergyDischargeWh = 0.0;


// Non-blocking timer for sensor reads
unsigned long lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL = 250; // Read sensors every 250ms


// Helper function to check for an I2C device ---
bool check_i2c_device(uint8_t address) {
  Wire.beginTransmission(address);
  // endTransmission returns 0 on success (device acknowledged)
  return (Wire.endTransmission() == 0);
}

void setup_power_monitor() {
  Serial.println("Initializing INA226 Sensor...");

  // --- Initialize the I2C bus FIRST ---
  Wire.begin();
  
  // Conditional Initialization for Channel 1 ---
  sensor_online[0] = check_i2c_device(INA226_CH1_ADDRESS);
  if (sensor_online[0]) {
    ina_ch1 = new Adafruit_INA219(INA226_CH1_ADDRESS);
    ina_ch1->setCalibration_32V_2A(); // Configure for 32V, 2A range
    ina_ch1->begin();
    // ina_ch1 = new INA226();
    // ina_ch1->begin(INA226_CH1_ADDRESS);
    // ina_ch1->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
    // ina_ch1->calibrate(INA226_CH1_SHUNT, 10);
    // Serial.println("INA226 Channel 1 (Solar Panel) Initialized.");
    Serial.println("INA219 Channel 1 (Solar Panel) Initialized.");
  } else {
    // Serial.println("INA226 Channel 1 not found.");
    Serial.println("INA219 Channel 1 not found.");
  }

  // Conditional Initialization for Channel 2 ---
  sensor_online[1] = check_i2c_device(INA226_CH2_ADDRESS);
  if (sensor_online[1]) {
    ina_ch2 = new INA226();
    ina_ch2->begin(INA226_CH2_ADDRESS);
    ina_ch2->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
    ina_ch2->calibrate(INA226_CH2_SHUNT, 10);
    Serial.println("INA226 Channel 2 (Battery) Initialized.");
  } else {
    Serial.println("INA226 Channel 2 not found.");
  }

  // Conditional Initialization for Channel 3 ---
  sensor_online[2] = check_i2c_device(INA226_CH3_ADDRESS);
  if (sensor_online[2]) {
    ina_ch3 = new INA226();
    ina_ch3->begin(INA226_CH3_ADDRESS);
    ina_ch3->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
    ina_ch3->calibrate(INA226_CH3_SHUNT, 10);
    Serial.println("INA226 Channel 3 (Load) Initialized.");
  } else {
    Serial.println("INA226 Channel 3 not found.");
  }
}

void loop_power_monitor() {
  if (millis() - lastSensorReadTime > SENSOR_READ_INTERVAL) {
    float timeDeltaHours = (float)SENSOR_READ_INTERVAL / 3600000.0; // (ms in interval) / (ms in hour)
    lastSensorReadTime = millis();
    char payloadBuffer[10]; // Reusable buffer for converting floats to strings
    
    // --- Read from Channel 1 ---
    if (ina_ch1 != nullptr) {
      busVoltage[0] = ina_ch1->getBusVoltage_V();   // readBusVoltage();
      current_ma[0] = ina_ch1->getCurrent_mA();     // readShuntCurrent() * 1000; // Convert Amps to Milliamps
      power_mw[0] = ina_ch1->getPower_mW();         // readBusPower() * 1000;       // Convert Watts to Milliwatts ---
      totalEnergyWh[0] += (power_mw[0] / 1000.0) * timeDeltaHours; // (Power in mW to W) * hours

      // Publish each measurement to its own topic
      dtostrf(busVoltage[0], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_SOLAR_PANEL_VOLTAGE_STATE, payloadBuffer, true);
      
      dtostrf(current_ma[0], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_SOLAR_PANEL_CURRENT_STATE, payloadBuffer, true);

      dtostrf(power_mw[0], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_SOLAR_PANEL_POWER_STATE, payloadBuffer, true);
      
      dtostrf(totalEnergyWh[0], 1, 4, payloadBuffer);
      client.publish(MQTT_TOPIC_SOLAR_PANEL_ENERGY_STATE, payloadBuffer, true);
    }

    // --- Read from Channel 2 ---
    if (ina_ch2 != nullptr) {
      busVoltage[1] = ina_ch2->readBusVoltage();
      current_ma[1] = ina_ch2->readShuntCurrent() * 1000; // Convert Amps to Milliamps
      power_mw[1] = ina_ch2->readBusPower() * 1000;       // Convert Watts to Milliwatts ---
      totalEnergyWh[1] += (power_mw[1] / 1000.0) * timeDeltaHours; // (Power in mW to W) * hours
      float batteryEnergyDeltaWh = (power_mw[1] / 1000.0) * timeDeltaHours; // Energy in Wh for this interval
      if (batteryEnergyDeltaWh > 0) {
        batteryEnergyChargeWh += batteryEnergyDeltaWh;  // Add to charge if positive
      } else {
        batteryEnergyDischargeWh += -batteryEnergyDeltaWh; // Add to discharge if negative
      }

      // Publish each measurement to its own topic
      dtostrf(busVoltage[1], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_BATTERY_VOLTAGE_STATE, payloadBuffer, true);
      
      dtostrf(current_ma[1], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_BATTERY_CURRENT_STATE, payloadBuffer, true);

      dtostrf(power_mw[1], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_BATTERY_POWER_STATE, payloadBuffer, true);
      
      dtostrf(batteryEnergyChargeWh, 1, 4, payloadBuffer);
      client.publish(MQTT_TOPIC_BATTERY_ENERGY_CHARGED_STATE, payloadBuffer, true);
      
      dtostrf(batteryEnergyDischargeWh, 1, 4, payloadBuffer);
      client.publish(MQTT_TOPIC_BATTERY_ENERGY_DISCHARGED_STATE, payloadBuffer, true);
    }

    // --- Read from Channel 3 ---
    if (ina_ch3 != nullptr) {
      busVoltage[2] = ina_ch3->readBusVoltage();
      current_ma[2] = ina_ch3->readShuntCurrent() * 1000; // Convert Amps to Milliamps
      power_mw[2] = ina_ch3->readBusPower() * 1000;       // Convert Watts to Milliwatts ---
      totalEnergyWh[2] += (power_mw[2] / 1000.0) * timeDeltaHours; // (Power in mW to W) * hours

      // Publish each measurement to its own topic
      dtostrf(busVoltage[2], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_LOAD_VOLTAGE_STATE, payloadBuffer, true);
      
      dtostrf(current_ma[2], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_LOAD_CURRENT_STATE, payloadBuffer, true);
      
      dtostrf(power_mw[2], 1, 2, payloadBuffer);
      client.publish(MQTT_TOPIC_LOAD_POWER_STATE, payloadBuffer, true);
      
      dtostrf(totalEnergyWh[2], 1, 4, payloadBuffer);
      client.publish(MQTT_TOPIC_LOAD_ENERGY_STATE, payloadBuffer, true);
    }
  }
}

// --- Data Getter Functions ---
float get_bus_voltage(int channel) {
  if (channel >= 1 && channel <= 3) return busVoltage[channel - 1];
  return 0.0;
}

float get_current(int channel) {
  if (channel >= 1 && channel <= 3) return current_ma[channel - 1];
  return 0.0;
}

float get_power(int channel) {
  if (channel >= 1 && channel <= 3) return power_mw[channel - 1];
  return 0.0;
}

bool is_sensor_online(int channel) {
  if (channel >= 1 && channel <= 3) return sensor_online[channel - 1];
  return false;
}