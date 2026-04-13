#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_wifi.h>
#include "config_portal.h"
#include "../StorageManager/storage.h"
#include "../../include/common.h"

static const char *TAG = "ConfigPortal";
static httpd_handle_t server = NULL;
static bool portal_active = false;

static const char *config_html PROGMEM = "<!DOCTYPE html>"
"<html><head><title>WiFi Config</title>"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<style>body{font-family:Arial;margin:40px;background:#f0f4f8} .form{max-width:400px;margin:auto;background:white;padding:30px;border-radius:10px;box-shadow:0 4px 12px rgba(0,0,0,0.1)} input,button{width:100%;padding:12px;margin:10px 0;font-size:16px;border:1px solid #ddd;border-radius:5px;box-sizing:border-box} button{background:#007cba;color:white;border:none;cursor:pointer} button:hover{background:#005a87} h1{text-align:center;color:#333}</style></head>"
"<body><div class=\"form\"><h1>ESP32 IoT Setup</h1>"
"<form action=\"/save\" method=\"post\">"
"<label>WiFi SSID:</label><input type=\"text\" name=\"ssid\" maxlength=\"32\" required><br>"
"<label>Password:</label><input type=\"password\" name=\"pass\" maxlength=\"64\"><br>"
"<button type=\"submit\">Connect & Reboot</button>"
"</form><p id=\"status\"></p></div>"
"<script>document.querySelector('form').onsubmit=()=>{document.getElementById('status').innerText='Saving...';}</script>"
"</body></html>";

static esp_err_t config_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, (const char *)config_html, strlen(config_html));
}

static esp_err_t save_post_handler(httpd_req_t *req) {
    char buf[100] = {0};
    int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
    char ssid[33] = {0};
    char pass[65] = {0};
    
    if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
        httpd_query_key_value(buf, "pass", pass, sizeof(pass));
        
        storage_init();
        if (storage_save_wifi(ssid, strlen(pass) ? pass : "") == ESP_OK) {
            httpd_resp_set_type(req, "text/html");
            httpd_resp_send(req, "<h1>Success!</h1><p>Rebooting in 3s...</p><script>setTimeout(()=>location='/',3000)</script>", -1);
            ESP_LOGI(TAG, "Config saved: %s", ssid);
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();
        } else {
            httpd_resp_send_404(req);
        }
    } else {
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

esp_err_t config_portal_start(void) {
    if (portal_active) return ESP_OK;
    
    // AP mode
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = 1,
            .password = WIFI_AP_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 2;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t config_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = config_get_handler
        };
        httpd_register_uri_handler(server, &config_uri);
        
        httpd_uri_t save_uri = {
            .uri = "/save",
            .method = HTTP_POST,
            .handler = save_post_handler
        };
        httpd_register_uri_handler(server, &save_uri);
        
        portal_active = true;
        ESP_LOGI(TAG, "Config portal started at 192.168.4.1");
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t config_portal_stop(void) {
    if (server) httpd_stop(server);
    esp_wifi_stop();
    portal_active = false;
    ESP_LOGI(TAG, "Config portal stopped");
    return ESP_OK;
}

bool config_portal_is_configured(void) {
    return portal_active;
}

