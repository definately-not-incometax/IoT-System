idf_component_register(SRCS "ota_handler.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_server esp_partition nvs_flash bootloader_support)

