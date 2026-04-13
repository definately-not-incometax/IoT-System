#include <esp_log.h>
#include <esp_random.h>
#include "cjson/cJSON.h"
#include "mock_sensor.h"

static const char *TAG = "MockSensor";

static bool mock_ready = false;
static uint32_t last_read = 0;

static esp_err_t mock_init(void) {
    ESP_LOGI(TAG, "Mock env sensor initialized");
    mock_ready = true;
    return ESP_OK;
}

static bool mock_is_ready(void) {
    return mock_ready;
}

static cJSON* mock_get_json_data(void) {
    cJSON *data = cJSON_CreateObject();
    // Realistic fake data
    float temp = 22.0 + 5.0 * sin((esp_timer_get_time() / 1e6) / 60.0) + (esp_random() % 100)/1000.0;
    float hum = 55.0 + 15.0 * cos((esp_timer_get_time() / 1e6) / 90.0) + (esp_random() % 200)/1000.0;
    
    cJSON_AddNumberToObject(data, "temperature", temp);
    cJSON_AddNumberToObject(data, "humidity", hum);
    
    ESP_LOGD(TAG, "Mock: temp=%.1fC hum=%.1f%%", temp, hum);
    return data;
}

static void mock_deinit(void) {
    mock_ready = false;
}

sensor_t mock_env_sensor = {
    .name = "environment",
    .init = mock_init,
    .is_ready = mock_is_ready,
    .get_json_data = mock_get_json_data,
    .deinit = mock_deinit
};

