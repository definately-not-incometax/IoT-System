#ifndef COMMON_H
#define COMMON_H

#define PROJECT_NAME "ESP32-IoT-Monitor"
#define DEVICE_ID "esp32_01"
#define WIFI_AP_SSID "Ruti Torkarii"
#define WIFI_AP_PASS "8274808564"
#define CONFIG_NAMESPACE "iot_config"

#define SENSOR_INTERVAL_MS 5000
#define HEARTBEAT_INTERVAL_S 30

// JSON field names match spec
#define JSON_DEVICE_ID "device_id"
#define JSON_TIMESTAMP "timestamp"
#define JSON_SENSORS "sensors"
#define JSON_SYSTEM "system"

// Log levels
#define LOG_LEVEL INFO

#endif // COMMON_H
