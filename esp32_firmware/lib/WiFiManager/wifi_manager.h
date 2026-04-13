#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <esp_err.h>

typedef enum {
    WIFI_STATE_INIT,
    WIFI_STATE_LOAD_CREDS,
    WIFI_STATE_STA_TRYING,
    WIFI_STATE_STA_CONNECTED,
    WIFI_STATE_AP_PORTAL,
} wifi_state_t;

typedef enum {
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_AP_STA
} wifi_mode_t;

void wifi_manager_init(void);
void wifi_manager_start(void);
esp_err_t wifi_manager_get_status(wifi_mode_t *mode, uint8_t *rssi);
wifi_state_t wifi_manager_get_state(void);
void wifi_task(void *pvParameters);

#endif

