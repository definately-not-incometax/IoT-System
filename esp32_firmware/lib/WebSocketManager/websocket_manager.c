#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_heap_caps.h"
#include "esp_http_server.h"
#include "cjson/cJSON.h"
#include "websocket_manager.h"
#include "../SensorManager/sensor_manager.h"
#include "../WiFiManager/wifi_manager.h"
#include "../include/common.h"

static const char *TAG = "WebSocketMgr";
static httpd_handle_t ws_server = NULL;
static int client_count = 0;

typedef struct {
    httpd_req_t *req;
    int id;
} ws_client_t;

static ws_client_t clients[10];
static int num_clients = 0;

static esp_err_t ws_open_handler(httpd_req_t *req) {
    if (num_clients < 10) {
        clients[num_clients].req = req;
        clients[num_clients].id = num_clients;
        num_clients++;
        client_count++;
        ESP_LOGI(TAG, "WS client #%d connected. Total: %d", clients[num_clients-1].id, client_count);
    }
    return ESP_OK;
}

static esp_err_t ws_close_handler(httpd_req_t *req) {
    // Find and remove
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].req == req) {
            ESP_LOGI(TAG, "WS client #%d disconnected", clients[i].id);
            // Shift array
            for (int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j+1];
            }
            num_clients--;
            client_count--;
            break;
        }
    }
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t *req) {
    // Handle WS frame
    uint8_t buf[128] = {0};
    int len = httpd_req_get_hdr_value_len(req, "Sec-WebSocket-Key") + 1;
    if (len > 0) {
        char key[len];
        httpd_req_get_hdr_value_str(req, "Sec-WebSocket-Key", key, len);
        // Simple handshake (production use full)
        httpd_resp_set_status(req, "101 Switching Protocols");
        httpd_resp_set_hdr(req, "Upgrade", "websocket");
        httpd_resp_set_hdr(req, "Connection", "Upgrade");
        httpd_resp_send(req, NULL, 0);
        ws_open_handler(req);
    } else {
        // Data frame (simplified, assume text)
        int data_len = httpd_req_recv(req, buf, sizeof(buf)-1);
        if (data_len > 0) {
            ESP_LOGD(TAG, "WS data: %.*s", data_len, buf);
            // Pong for ping
            if (buf[0] == 0x89) { // Ping
                uint8_t pong[128];
                pong[0] = 0x8A; // Pong opcode
                memcpy(pong + 2, buf + 2, data_len - 2);
                httpd_ws_resp_send(req, pong, data_len);
            }
        }
    }
    return ESP_OK;
}

static void broadcast_data(void) {
    if (num_clients == 0) return;
    
    char *sensor_json = sensor_manager_get_json_str();
    if (!sensor_json) return;
    
    // Add system info
    cJSON *data = cJSON_Parse(sensor_json);
    free(sensor_json);
    cJSON *system_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(system_obj, "uptime", esp_timer_get_time() / 1000);
    uint8_t rssi;
    wifi_mode_t mode;
    wifi_manager_get_status(&mode, &rssi);
    cJSON_AddNumberToObject(system_obj, "rssi", rssi);
    cJSON_AddNumberToObject(system_obj, "heap", esp_get_free_heap_size());
    cJSON_AddItemToObject(data, "system", system_obj);
    
    char *full_json = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    
    // Send to all clients
    for (int i = 0; i < num_clients; i++) {
        httpd_ws_resp_send(clients[i].req, (uint8_t*)full_json, strlen(full_json));
    }
    ESP_LOGI(TAG, "Broadcast to %d clients: %s", num_clients, full_json);
    free(full_json);
}

static void ws_task(void *pvParameters) {
    ESP_LOGI(TAG, "WebSocket task started");
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 81;
    
    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL
    };
    
    if (httpd_start(&ws_server, &config) == ESP_OK) {
        httpd_register_uri_handler(ws_server, &ws_uri);
    }
    
    uint64_t last_broadcast = 0;
    while (1) {
        uint64_t now = esp_timer_get_time() / 1000;
        if (now - last_broadcast > 2000) {
            broadcast_data();
            last_broadcast = now;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void websocket_manager_start(void) {
    xTaskCreate(ws_task, "WSTask", 8192, NULL, 3, NULL);
}

