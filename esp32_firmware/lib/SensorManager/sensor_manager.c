#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "cjson/cJSON.h"
#include "sensor_manager.h"
#include "../include/common.h"
#include "../MockSensor/mock_sensor.h"

static const char *TAG = "SensorManager";
static TaskHandle_t sensor_task_handle = NULL;
static cJSON *sensor_data = NULL;
static uint64_t last_poll = 0;
static const uint32_t POLL_INTERVAL_MS = 2000;

static sensor_t *registered_sensors[8];
static int num_sensors = 0;

esp_err_t sensor_register(sensor_t *sensor) {
    if (num_sensors >= 8) return ESP_ERR_NO_MEM;
    registered_sensors[num_sensors++] = sensor;
    ESP_LOGI(TAG, "Registered sensor: %s", sensor->name);
    return ESP_OK;
}

void sensor_manager_init(void) {
    sensor_data = cJSON_CreateObject();
    cJSON_AddStringToObject(sensor_data, JSON_DEVICE_ID, DEVICE_ID);
    
    // Register default sensors
    sensor_register(&mock_env_sensor);
    // Add more: sensor_register(&dht_sensor);
    
    ESP_LOGI(TAG, "Sensor manager initialized with %d sensors", num_sensors);
}

bool sensor_manager_should_poll(void) {
    uint64_t now = esp_timer_get_time() / 1000;
    return (now - last_poll) >= POLL_INTERVAL_MS;
}

void sensor_manager_poll(void) {
    last_poll = esp_timer_get_time() / 1000;
    
    cJSON *sensors_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(sensor_data, JSON_SENSORS, sensors_obj);
    
    for (int i = 0; i < num_sensors; i++) {
        sensor_t *s = registered_sensors[i];
        if (s->is_ready()) {
            cJSON *data = s->get_json_data();
            if (data) {
                cJSON_AddItemToObject(sensors_obj, s->name, data);
            }
        }
    }
    
    cJSON_AddNumberToObject(sensor_data, JSON_TIMESTAMP, last_poll);
}

const char* sensor_manager_get_json_str(void) {
    if (!sensor_data) return NULL;
    char *str = cJSON_PrintUnformatted(sensor_data);
    return str;  // Caller must free
}

void sensor_task(void *pvParameters) {
    sensor_manager_init();
    
    while (1) {
        if (sensor_manager_should_poll()) {
            sensor_manager_poll();
            ESP_LOGI(TAG, "Sensor data updated");
            char *json_str = sensor_manager_get_json_str();
            ESP_LOGI(TAG, "Data: %s", json_str);
            free(json_str);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void sensor_manager_start(void) {
    xTaskCreate(sensor_task, "SensorTask", 4096, NULL, 4, &sensor_task_handle);
}

