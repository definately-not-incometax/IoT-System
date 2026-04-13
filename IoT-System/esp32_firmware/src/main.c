#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "../lib/WiFiManager/wifi_manager.h"
#include "../include/common.h"

static const char *TAG = PROJECT_NAME;

void app_main(void)
{
    ESP_LOGI(TAG, "%s PHASE 3: Sensor Layer integrated. v0.3.0", PROJECT_NAME);

    wifi_manager_start();
    
    sensor_manager_start();
    
    websocket_manager_start();
    
    ota_handler_start();

    ESP_LOGI(TAG, "All tasks launched (WiFi, Sensor, WS, OTA).");

    // Supervisor loop
    uint32_t counter = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        wifi_mode_t mode;
        uint8_t rssi;
        wifi_manager_get_status(&mode, &rssi);
        ESP_LOGI(TAG, "Supervisor tick #%ld WiFi: %d rssi:%d", ++counter, mode, rssi);
    }
}


