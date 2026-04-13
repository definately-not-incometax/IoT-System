#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <esp_err.h>

esp_err_t storage_init(void);
esp_err_t storage_save_wifi(const char* ssid, const char* password);
esp_err_t storage_load_wifi(char* ssid, size_t ssid_len, char* password, size_t pass_len);
bool storage_has_wifi_creds(void);

#endif

