idf_component_register(SRCS "main.c"
                        REQUIRES driver esp_event esp_wifi esp_http_client nvs_flash esp_timer bsp
                       INCLUDE_DIRS "")
