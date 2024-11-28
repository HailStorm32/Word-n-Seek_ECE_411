    idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client nvs_flash esp_wifi) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirementsments