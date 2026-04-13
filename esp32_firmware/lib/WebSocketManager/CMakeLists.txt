idf_component_register(SRCS "websocket_manager.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_server esp_websocket_sensor_manager WiFiManager)

