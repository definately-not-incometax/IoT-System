#ifndef CONFIG_PORTAL_H
#define CONFIG_PORTAL_H

#include <esp_err.h>

esp_err_t config_portal_start(void);
esp_err_t config_portal_stop(void);
bool config_portal_is_configured(void);

#endif

