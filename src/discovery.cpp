#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "discovery.h"
#include "config.h"

// This function needs access to the global MQTT client object
extern PubSubClient client;

void mqtt_discovery() {

    // Build and publish discovery json for the device and all components
    JsonDocument discovery_doc;
    const char* discovery_topic = "homeassistant/device/shed_power_monitor/config"; // Unique topic for this device

    // Device document
    JsonObject device_doc = discovery_doc["device"].to<JsonObject>();
    device_doc["name"] = "Shed Solar Monitor";
    device_doc["ids"] = DEVICE_ID;  // shed_solar_power_monitor
    device_doc["mf"] = "Psyki Heavy Industries - Gem Systems";
    device_doc["mdl"] = "ESP32 Solar Core";
    device_doc["suggested_area"] = "Shed";

    // Origin document
    JsonObject origin_doc = discovery_doc["o"].to<JsonObject>();
    origin_doc["name"] = "Psyki + Gem 100 years";
    origin_doc["sw"] = "0.2";
    origin_doc["url"] = "https://switz.org";

    JsonObject cmps_doc = discovery_doc["cmps"].to<JsonObject>();

    // Motion activated light timer
    JsonObject motion_timer_cmp = cmps_doc["shed_light_motion_timer"].to<JsonObject>();
    motion_timer_cmp["name"] = "Shed Motion Timer";
    motion_timer_cmp["p"] = "number";
    motion_timer_cmp["min"] = 10;
    motion_timer_cmp["max"] = 3600;
    motion_timer_cmp["unit_of_meas"] = "s";
    motion_timer_cmp["uniq_id"] = "shed_solar_monitor_light_motion_timer";
    motion_timer_cmp["object_id"] = "shed_light_motion_timer";
    motion_timer_cmp["~"] = MQTT_BASE_TOPIC_MOTION_TIMER; 		// home/shed/number/motion_timer
    motion_timer_cmp["stat_t"] = "~/state";                     // home/shed/number/motion_timer/state
    motion_timer_cmp["cmd_t"] = "~/command";                    // home/shed/number/motion_timer/command
    motion_timer_cmp["avty_t"] = MQTT_TOPIC_DEVICE_AVAILABILITY;       // devices/shed/solar_power_monitor/status

    // Manual override light timer
    JsonObject manual_timer_cmp = cmps_doc["shed_light_override_timer"].to<JsonObject>();
    manual_timer_cmp["name"] = "Shed Override Timer";
    manual_timer_cmp["p"] = "number";
    manual_timer_cmp["min"] = 10;
    manual_timer_cmp["max"] = 3600;
    manual_timer_cmp["unit_of_meas"] = "s";
    manual_timer_cmp["uniq_id"] = "shed_solar_monitor_light_override_timer";
    manual_timer_cmp["object_id"] = "shed_light_override_timer";
    manual_timer_cmp["~"] = MQTT_BASE_TOPIC_MANUAL_TIMER; 		// home/shed/number/manual_timer
    manual_timer_cmp["stat_t"] = "~/state";                     // home/shed/number/manual_timer/state
    manual_timer_cmp["cmd_t"] = "~/command";                    // home/shed/number/manual_timer/command
    manual_timer_cmp["avty_t"] = MQTT_TOPIC_DEVICE_AVAILABILITY;       // devices/shed/solar_power_monitor/status

    // INA226 Channel 1 - Solar Panel Voltage
    JsonObject power_ch1_v_cmp = cmps_doc["shed_solar_monitor_ch1_voltage"].to<JsonObject>();
    power_ch1_v_cmp["name"] = "Solar Panel Voltage";
    power_ch1_v_cmp["p"] = "sensor";
    power_ch1_v_cmp["dev_cla"] = "voltage";
    power_ch1_v_cmp["unit_of_meas"] = "V";
    power_ch1_v_cmp["stat_cla"] = "measurement";
    power_ch1_v_cmp["val_tpl"] = "{{ value | float }}";
    power_ch1_v_cmp["uniq_id"] = "shed_solar_monitor_ch1_voltage";
    power_ch1_v_cmp["object_id"] = "shed_solar_panel_voltage";
    power_ch1_v_cmp["ic"] = "mdi:flash";
    power_ch1_v_cmp["stat_t"] = MQTT_TOPIC_SOLAR_PANEL_VOLTAGE_STATE;	// home/shed/sensor/solar_panel_voltage/state
    power_ch1_v_cmp["avty_t"] = MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/panel_sensor_status
    power_ch1_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch1_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 1 - Solar Panel Current
    JsonObject power_ch1_a_cmp = cmps_doc["shed_solar_monitor_ch1_current"].to<JsonObject>();
    power_ch1_a_cmp["name"] = "Solar Panel Current";
    power_ch1_a_cmp["p"] = "sensor";
    power_ch1_a_cmp["dev_cla"] = "current";
    power_ch1_a_cmp["unit_of_meas"] = "mA";
    power_ch1_a_cmp["stat_cla"] = "measurement";
    power_ch1_a_cmp["val_tpl"] = "{{ value | float }}";
    power_ch1_a_cmp["uniq_id"] = "shed_solar_monitor_ch1_current";
    power_ch1_a_cmp["object_id"] = "shed_solar_panel_current";
    power_ch1_a_cmp["ic"] = "mdi:current-dc";
    power_ch1_a_cmp["stat_t"] = MQTT_TOPIC_SOLAR_PANEL_CURRENT_STATE;	// home/shed/sensor/solar_panel_current/state
    power_ch1_a_cmp["avty_t"] = MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/panel_sensor_status
    power_ch1_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch1_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 1 - Solar Panel Power
    JsonObject power_ch1_p_cmp = cmps_doc["shed_solar_monitor_ch1_power"].to<JsonObject>();
    power_ch1_p_cmp["name"] = "Solar Panel Power";
    power_ch1_p_cmp["p"] = "sensor";
    power_ch1_p_cmp["dev_cla"] = "power";
    power_ch1_p_cmp["unit_of_meas"] = "mw";
    power_ch1_p_cmp["stat_cla"] = "measurement";
    power_ch1_p_cmp["val_tpl"] = "{{ value | float }}";
    power_ch1_p_cmp["uniq_id"] = "shed_solar_monitor_ch1_power";
    power_ch1_p_cmp["object_id"] = "shed_solar_panel_power";
    power_ch1_p_cmp["ic"] = "mdi:solar-power-variant";
    power_ch1_p_cmp["stat_t"] = MQTT_TOPIC_SOLAR_PANEL_POWER_STATE;		// home/shed/sensor/solar_panel_power/state
    power_ch1_p_cmp["avty_t"] = MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/panel_sensor_status
    power_ch1_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch1_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 1 - Solar Panel Total Energy
    JsonObject power_ch1_e_cmp = cmps_doc["shed_solar_monitor_ch1_energy"].to<JsonObject>();
    power_ch1_e_cmp["name"] = "Solar Panel Energy";
    power_ch1_e_cmp["p"] = "sensor";
    power_ch1_e_cmp["dev_cla"] = "energy";
    power_ch1_e_cmp["unit_of_meas"] = "Wh";
    power_ch1_e_cmp["stat_cla"] = "total_increasing";
    power_ch1_e_cmp["val_tpl"] = "{{ value | float }}";
    power_ch1_e_cmp["uniq_id"] = "shed_solar_monitor_ch1_energy";
    power_ch1_e_cmp["object_id"] = "shed_solar_panel_energy";
    power_ch1_e_cmp["ic"] = "mdi:chart-histogram";
    power_ch1_e_cmp["stat_t"] = MQTT_TOPIC_SOLAR_PANEL_ENERGY_STATE;	// home/shed/sensor/solar_panel_energy/state
    power_ch1_e_cmp["avty_t"] = MQTT_TOPIC_PANEL_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/panel_sensor_status
    power_ch1_e_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch1_e_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Voltage
    JsonObject power_ch2_v_cmp = cmps_doc["shed_solar_monitor_ch2_voltage"].to<JsonObject>();
    power_ch2_v_cmp["name"] = "Battery Voltage";
    power_ch2_v_cmp["p"] = "sensor";
    power_ch2_v_cmp["dev_cla"] = "voltage";
    power_ch2_v_cmp["unit_of_meas"] = "V";
    power_ch2_v_cmp["stat_cla"] = "measurement";
    power_ch2_v_cmp["val_tpl"] = "{{ value | float }}";
    power_ch2_v_cmp["uniq_id"] = "shed_solar_monitor_ch2_voltage";
    power_ch2_v_cmp["object_id"] = "shed_battery_voltage";
    power_ch2_v_cmp["ic"] = "mdi:flash";
    power_ch2_v_cmp["stat_t"] = MQTT_TOPIC_BATTERY_VOLTAGE_STATE;		// home/shed/sensor/solar_battery_voltage/state
    power_ch2_v_cmp["avty_t"] = MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/battery_sensor_status
    power_ch2_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch2_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Current
    JsonObject power_ch2_a_cmp = cmps_doc["shed_solar_monitor_ch2_current"].to<JsonObject>();
    power_ch2_a_cmp["name"] = "Battery Current";
    power_ch2_a_cmp["p"] = "sensor";
    power_ch2_a_cmp["dev_cla"] = "current";
    power_ch2_a_cmp["unit_of_meas"] = "mA";
    power_ch2_a_cmp["stat_cla"] = "measurement";
    power_ch2_a_cmp["val_tpl"] = "{{ value | float }}";
    power_ch2_a_cmp["uniq_id"] = "shed_solar_monitor_ch2_current";
    power_ch2_a_cmp["object_id"] = "shed_battery_current";
    power_ch2_a_cmp["ic"] = "mdi:current-dc";
    power_ch2_a_cmp["stat_t"] = MQTT_TOPIC_BATTERY_CURRENT_STATE;		// home/shed/sensor/solar_battery_current/state
    power_ch2_a_cmp["avty_t"] = MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/battery_sensor_status
    power_ch2_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch2_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Power
    JsonObject power_ch2_p_cmp = cmps_doc["shed_solar_monitor_ch2_power"].to<JsonObject>();
    power_ch2_p_cmp["name"] = "Battery Power";
    power_ch2_p_cmp["p"] = "sensor";
    power_ch2_p_cmp["dev_cla"] = "power";
    power_ch2_p_cmp["unit_of_meas"] = "mw";
    power_ch2_p_cmp["stat_cla"] = "measurement";
    power_ch2_p_cmp["val_tpl"] = "{{ value | float }}";
    power_ch2_p_cmp["uniq_id"] = "shed_solar_monitor_ch2_power";
    power_ch2_p_cmp["object_id"] = "shed_battery_power";
    power_ch2_p_cmp["ic"] = "mdi:battery";
    power_ch2_p_cmp["stat_t"] = MQTT_TOPIC_BATTERY_POWER_STATE;			// home/shed/sensor/solar_battery_power/state
    power_ch2_p_cmp["avty_t"] = MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/battery_sensor_status
    power_ch2_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch2_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Energy Charged
    JsonObject power_ch2_e_in_cmp = cmps_doc["shed_solar_monitor_ch2_energy_in"].to<JsonObject>();
    power_ch2_e_in_cmp["name"] = "Battery Energy Charged";
    power_ch2_e_in_cmp["p"] = "sensor";
    power_ch2_e_in_cmp["dev_cla"] = "energy";
    power_ch2_e_in_cmp["unit_of_meas"] = "Wh";
    power_ch2_e_in_cmp["stat_cla"] = "total_increasing";
    power_ch2_e_in_cmp["val_tpl"] = "{{ value | float }}";
    power_ch2_e_in_cmp["uniq_id"] = "shed_solar_monitor_ch2_energy_in";
    power_ch2_e_in_cmp["object_id"] = "shed_battery_energy_charged";
    power_ch2_e_in_cmp["ic"] = "mdi:battery-arrow-up";
    power_ch2_e_in_cmp["stat_t"] = MQTT_TOPIC_BATTERY_ENERGY_CHARGED_STATE;	// home/shed/sensor/solar_battery_energy_charged/state
    power_ch2_e_in_cmp["avty_t"] = MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/battery_sensor_status
    power_ch2_e_in_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch2_e_in_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Energy Discharged
    JsonObject power_ch2_e_out_cmp = cmps_doc["shed_solar_monitor_ch2_energy_out"].to<JsonObject>();
    power_ch2_e_out_cmp["name"] = "Battery Energy Discharged";
    power_ch2_e_out_cmp["p"] = "sensor";
    power_ch2_e_out_cmp["dev_cla"] = "energy";
    power_ch2_e_out_cmp["unit_of_meas"] = "Wh";
    power_ch2_e_out_cmp["stat_cla"] = "total_increasing";
    power_ch2_e_out_cmp["val_tpl"] = "{{ value | float }}";
    power_ch2_e_out_cmp["uniq_id"] = "shed_solar_monitor_ch2_energy_out";
    power_ch2_e_out_cmp["object_id"] = "shed_battery_energy_discharged";
    power_ch2_e_out_cmp["ic"] = "mdi:battery-arrow-down";
    power_ch2_e_out_cmp["stat_t"] = MQTT_TOPIC_BATTERY_ENERGY_DISCHARGED_STATE;	// home/shed/sensor/solar_battery_energy_discharged/state
    power_ch2_e_out_cmp["avty_t"] = MQTT_TOPIC_BATTERY_SENSOR_AVAILABILITY;		// devices/shed_power_monitor/battery_sensor_status
    power_ch2_e_out_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch2_e_out_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 3 - Load Voltage
    JsonObject power_ch3_v_cmp = cmps_doc["shed_solar_monitor_ch3_voltage"].to<JsonObject>();
    power_ch3_v_cmp["name"] = "Load Voltage";
    power_ch3_v_cmp["p"] = "sensor";
    power_ch3_v_cmp["dev_cla"] = "voltage";
    power_ch3_v_cmp["unit_of_meas"] = "V";
    power_ch3_v_cmp["stat_cla"] = "measurement";
    power_ch3_v_cmp["val_tpl"] = "{{ value | float }}";
    power_ch3_v_cmp["uniq_id"] = "shed_solar_monitor_ch3_voltage";
    power_ch3_v_cmp["object_id"] = "shed_load_voltage";
    power_ch3_v_cmp["ic"] = "mdi:flash";
    power_ch3_v_cmp["stat_t"] = MQTT_TOPIC_LOAD_VOLTAGE_STATE;			// home/shed/sensor/solar_load_voltage/state
    power_ch3_v_cmp["avty_t"] = MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/load_sensor_status
    power_ch3_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch3_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 3 - Load Current
    JsonObject power_ch3_a_cmp = cmps_doc["shed_solar_monitor_ch3_current"].to<JsonObject>();
    power_ch3_a_cmp["name"] = "Load Current";
    power_ch3_a_cmp["p"] = "sensor";
    power_ch3_a_cmp["dev_cla"] = "current";
    power_ch3_a_cmp["unit_of_meas"] = "mA";
    power_ch3_a_cmp["stat_cla"] = "measurement";
    power_ch3_a_cmp["val_tpl"] = "{{ value | float }}";
    power_ch3_a_cmp["uniq_id"] = "shed_solar_monitor_ch3_current";
    power_ch3_a_cmp["object_id"] = "shed_load_current";
    power_ch3_a_cmp["ic"] = "mdi:current-dc";
    power_ch3_a_cmp["stat_t"] = MQTT_TOPIC_LOAD_CURRENT_STATE;			// home/shed/sensor/solar_load_current/state
    power_ch3_a_cmp["avty_t"] = MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/load_sensor_status
    power_ch3_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch3_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 3 - Load Power
    JsonObject power_ch3_p_cmp = cmps_doc["shed_solar_monitor_ch3_power"].to<JsonObject>();
    power_ch3_p_cmp["name"] = "Load Power";
    power_ch3_p_cmp["p"] = "sensor";
    power_ch3_p_cmp["dev_cla"] = "power";
    power_ch3_p_cmp["unit_of_meas"] = "mW";
    power_ch3_p_cmp["stat_cla"] = "measurement";
    power_ch3_p_cmp["val_tpl"] = "{{ value | float }}";
    power_ch3_p_cmp["uniq_id"] = "shed_solar_monitor_ch3_power";
    power_ch3_p_cmp["object_id"] = "shed_load_power";
    power_ch3_p_cmp["ic"] = "mdi:power-plug";
    power_ch3_p_cmp["stat_t"] = MQTT_TOPIC_LOAD_POWER_STATE;			// home/shed/sensor/solar_load_power/state
    power_ch3_p_cmp["avty_t"] = MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/load_sensor_status
    power_ch3_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch3_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 3 - Load Total Energy
    JsonObject power_ch3_e_cmp = cmps_doc["shed_solar_monitor_ch3_energy"].to<JsonObject>();
    power_ch3_e_cmp["name"] = "Load Energy";
    power_ch3_e_cmp["p"] = "sensor";
    power_ch3_e_cmp["dev_cla"] = "energy";
    power_ch3_e_cmp["unit_of_meas"] = "Wh";
    power_ch3_e_cmp["stat_cla"] = "total_increasing";
    power_ch3_e_cmp["val_tpl"] = "{{ value | float }}";
    power_ch3_e_cmp["uniq_id"] = "shed_solar_monitor_ch3_energy";
    power_ch3_e_cmp["object_id"] = "shed_load_energy";
    power_ch3_e_cmp["ic"] = "mdi:chart-histogram";
    power_ch3_e_cmp["stat_t"] = MQTT_TOPIC_LOAD_ENERGY_STATE;			// home/shed/sensor/solar_load_energy/state
    power_ch3_e_cmp["avty_t"] = MQTT_TOPIC_LOAD_SENSOR_AVAILABILITY;	// devices/shed_power_monitor/load_sensor_status
    power_ch3_e_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
    power_ch3_e_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // Print the JSON document to the Serial console for debugging
    Serial.println("--- Device Discovery Payload: ---");
    serializeJsonPretty(discovery_doc, Serial);
    Serial.println();
    Serial.print("Discovery topic:");
    Serial.println(discovery_topic);

    // Print the total size of the JSON document
    size_t jsonSize = measureJson(discovery_doc);
    Serial.println("--------------------------------");
    Serial.print("Total JSON size: ");
    Serial.println(jsonSize);
    Serial.println("--------------------------------");

    if (jsonSize < DEVICE_DISCOVERY_PAYLOAD_SIZE) {
        Serial.println("Publishing discovery document to MQTT broker...");
        static char buffer[DEVICE_DISCOVERY_PAYLOAD_SIZE];
        serializeJson(discovery_doc, buffer);
        client.publish(discovery_topic, buffer, true);
    } else {
        Serial.println("Error: JSON document size exceeds buffer size.");
    }

}