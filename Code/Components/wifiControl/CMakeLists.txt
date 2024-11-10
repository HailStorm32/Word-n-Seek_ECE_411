idf_component_register(SRCS "main.c" "wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_wifi) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements