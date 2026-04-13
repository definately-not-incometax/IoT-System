#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_flash_partitions.h>
#include "ota_handler.h"
#include "../include/common.h"

static const char *TAG = "OTAHandler";
static httpd_handle_t ota_server = NULL;

static const char *ota_html = "<!DOCTYPE html>"
"<html><head><title>OTA Update</title><meta name=\"viewport\" content=\"width=device-width\">"
"<style>body{font-family:Arial;margin:40px;background:#f5f5f5}.card{max-width:500px;margin:auto;background:white;padding:30px;border-radius:10px;box-shadow:0 4px 12px rgba(0,0,0,0.1)}input,button{width:100%;padding:15px;margin:10px 0;font-size:16px;border:1px solid #ddd;border-radius:5px}button{background:#28a745;color:white;border:none;cursor:pointer}button:hover{background:#218838}#progress{display:none;background:#e9ecef;height:20px;border-radius:10px;overflow:hidden}#bar{background:#28a745;height:100%;width:0%;transition:width .3s}</style></head>"
"<body><div class=\"card\"><h1>ESP32 IoT OTA Update</h1>"
"<form id=\"otaForm\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"update\" accept=\".bin\" required><button type=\"submit\">Update Firmware</button></form>"
"<div id=\"progress\"><div id=\"bar\"></div></div><p id=\"status\"></p></div>"
"<script>document.getElementById('otaForm').onsubmit = async e => { e.preventDefault(); const formData = new FormData(e.target); const xhr = new XMLHttpRequest(); xhr.open('POST', '/update'); xhr.upload.onprogress = e => { const pct = e.loaded / e.total * 100; document.getElementById('bar').style.width = pct + '%'; }; xhr.onload = () => { document.getElementById('status').textContent = xhr.status === 200 ? 'Update success! Rebooting...' : 'Update failed'; }; xhr.send(formData); document.getElementById('progress').style.display = 'block'; };</script>"
"</body></html>";

static esp_err_t ota_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, ota_html, strlen(ota_html));
}

static esp_err_t ota_update_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "OTA update started");
    
    const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
    if (!ota_partition) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition");
        ESP_LOGE(TAG, "No OTA partition");
        return ESP_FAIL;
    }
    
    esp_ota_handle_t ota_handle;
    esp_err_t err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA begin failed");
        ESP_LOGE(TAG, "OTA begin failed: %s", esp_err_to_name(err));
        return err;
    }
    
    int total_len = 0;
    int received = 0;
    char buf[1024];
    
    while (1) {
        int len = httpd_req_recv(req, buf, sizeof(buf));
        if (len < 0) {
            ESP_LOGE(TAG, "Recv failed");
            break;
        }
        if (len == 0) break;
        
        received += len;
        err = esp_ota_write(ota_handle, (const void *)buf, len);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "OTA write failed: %s", esp_err_to_name(err));
            break;
        }
        
        total_len += len;
        int pct = (total_len * 100) / total_len; // Approximate
        ESP_LOGI(TAG, "OTA progress: %d bytes ( ~%d%% )", total_len, pct);
    }
    
    if (esp_ota_end(ota_handle) != ESP_OK) {
        const esp_ota_img_states_t ota_state;
        if (esp_ota_get_state_partition(ota_partition, &ota_state) == ESP_OK) {
            if (ota_state == ESP_OTA_IMG_NEW) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA corrupt");
                ESP_LOGE(TAG, "OTA image corrupt");
                return ESP_FAIL;
            }
        }
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA end failed");
        return ESP_FAIL;
    }
    
    err = esp_ota_set_boot_partition(ota_partition);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Set boot failed");
        ESP_LOGE(TAG, "Set boot partition failed");
        return err;
    }
    
    httpd_resp_send(req, "OTA Success! Rebooting...", -1);
    ESP_LOGI(TAG, "OTA success, rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

void ota_handler_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.lru_purge_enable = true;
    
    httpd_uri_t ota_uri = {
        .uri = "/ota",
        .method = HTTP_GET,
        .handler = ota_get_handler,
    };
    
    httpd_uri_t update_uri = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = ota_update_handler,
    };
    
    if (httpd_start(&ota_server, &config) == ESP_OK) {
        httpd_register_uri_handler(ota_server, &ota_uri);
        httpd_register_uri_handler(ota_server, &update_uri);
        ESP_LOGI(TAG, "OTA server started on port 80");
    }
}

