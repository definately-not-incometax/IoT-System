#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <esp_err.h>
#include "cjson/cJSON.h"

typedef struct {
    const char* name;
    esp_err_t (*init)(void);
    bool (*is_ready)(void);
    cJSON* (*get_json_data)(void);
    void (*deinit)(void);
} sensor_t;

void sensor_manager_init(void);
void sensor_manager_start(void);
const char* sensor_manager_get_json_str(void);
void sensor_task(void *pvParameters);

#endif

