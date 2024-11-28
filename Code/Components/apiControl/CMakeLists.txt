idf_component_register(SRCS "api_client.c"
                    INCLUDE_DIRS "include"
                    REQUIRES log esp_http_client) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirementsments
                    