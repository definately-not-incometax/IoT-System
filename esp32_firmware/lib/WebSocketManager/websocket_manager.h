#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <esp_err.h>

void websocket_manager_init(void);
void websocket_manager_start(void);
void websocket_task(void *pvParameters);

#endif

