#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "storage.h"
#include "../include/common.h"

static const char *TAG = "Storage";
static nvs_handle_t prefs_handle;

esp_err_t storage_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    
    err = nvs_open_from_partition("nvs", CONFIG_NAMESPACE, NVS_READWRITE, &prefs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Storage initialized");
    return ESP_OK;
}

esp_err_t storage_save_wifi(const char* ssid, const char* password) {
    esp_err_t err;
    err = nvs_set_str(prefs_handle, "wifi_ssid", ssid);
    if (err != ESP_OK) return err;
    err = nvs_set_str(prefs_handle, "wifi_pass", password);
    if (err != ESP_OK) return err;
    err = nvs_commit(prefs_handle);
    ESP_LOGI(TAG, "WiFi creds saved");
    return err;
}

esp_err_t storage_load_wifi(char* ssid, size_t ssid_len, char* password, size_t pass_len) {
    esp_err_t err1 = nvs_get_str(prefs_handle, "wifi_ssid", ssid, &ssid_len);
    esp_err_t err2 = nvs_get_str(prefs_handle, "wifi_pass", password, &pass_len);
    if (err1 == ESP_OK && err2 == ESP_OK) {
        ESP_LOGI(TAG, "WiFi creds loaded");
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}

bool storage_has_wifi_creds(void) {
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(prefs_handle, "wifi_ssid", NULL, &required_size);
    return (err == ESP_OK && required_size > 0);
}


