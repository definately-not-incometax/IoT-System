#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "../include/common.h"

static const char *TAG = "WiFiManager";
static bool wifi_initialized = false;

static wifi_state_t current_state = WIFI_STATE_INIT;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    switch (event_base) {
        case WIFI_EVENT:
            if (event_id == WIFI_EVENT_STA_START) {
                ESP_LOGI(TAG, "WiFi STA started, connecting...");
            } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
                ESP_LOGW(TAG, "WiFi STA disconnected");
                current_state = WIFI_STATE_STA_TRYING;
            }
            break;
        case IP_EVENT:
            if (event_id == IP_EVENT_STA_GOT_IP) {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "WiFi connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
                current_state = WIFI_STATE_STA_CONNECTED;
            }
            break;
        default:
            ESP_LOGI(TAG, "WiFi event: %s %ld", event_base, event_id);
    }
}

void wifi_manager_init(void) {
    if (wifi_initialized) return;
    
    storage_init();  // Init NVS for storage
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialized");
}

void wifi_manager_start(void) {
    wifi_manager_init();
    xTaskCreate(wifi_task, "WiFiTask", 4096, NULL, 5, NULL);
}

esp_err_t wifi_manager_get_status(wifi_mode_t *mode, uint8_t *rssi) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        *rssi = ap_info.rssi;
        *mode = WIFI_MODE_STA;
    } else {
        *mode = WIFI_MODE_AP;
        *rssi = 0;
    }
    return ESP_OK;
}

wifi_state_t wifi_manager_get_state(void) {
    return current_state;
}

static int connect_attempts = 0;
static const int MAX_CONNECT_ATTEMPTS = 3;
static const int CONNECT_TIMEOUT_MS = 15000;

void wifi_task(void *pvParameters) {
    ESP_LOGI(TAG, "WiFi task started");
    
    wifi_manager_init();
    
    while (1) {
        switch (current_state) {
            case WIFI_STATE_INIT:
                current_state = WIFI_STATE_LOAD_CREDS;
                break;
                
            case WIFI_STATE_LOAD_CREDS:
                if (storage_has_wifi_creds()) {
                    char ssid[33], pass[65];
                    if (storage_load_wifi(ssid, sizeof(ssid), pass, sizeof(pass)) == ESP_OK) {
                        wifi_config_t wifi_config = { .sta = { .ssid = {0}, .password = {0} } };
                        strcpy((char*)wifi_config.sta.ssid, ssid);
                        strcpy((char*)wifi_config.sta.password, pass);
                        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
                        ESP_ERROR_CHECK(esp_wifi_start());
                        current_state = WIFI_STATE_STA_TRYING;
                        connect_attempts = 0;
                        ESP_LOGI(TAG, "Trying STA with saved creds");
                    }
                } else {
                    ESP_LOGI(TAG, "No creds, starting config portal");
                    current_state = WIFI_STATE_AP_PORTAL;
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
                
            case WIFI_STATE_STA_TRYING:
                connect_attempts++;
                if (connect_attempts > MAX_CONNECT_ATTEMPTS) {
                    ESP_LOGW(TAG, "STA connect failed, starting portal");
                    esp_wifi_stop();
                    config_portal_start();
                    current_state = WIFI_STATE_AP_PORTAL;
                    connect_attempts = 0;
                }
                ESP_LOGI(TAG, "STA trying... attempt %d/%d", connect_attempts, MAX_CONNECT_ATTEMPTS);
                vTaskDelay(pdMS_TO_TICKS(CONNECT_TIMEOUT_MS / MAX_CONNECT_ATTEMPTS));
                break;
                
            case WIFI_STATE_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi healthy");
                vTaskDelay(pdMS_TO_TICKS(30000));
                break;
                
            case WIFI_STATE_AP_PORTAL:
                ESP_LOGI(TAG, "Waiting config portal save/reboot");
                vTaskDelay(pdMS_TO_TICKS(5000));
                break;
                
            default:
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
        }
    }
}


