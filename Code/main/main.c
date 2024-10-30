// #include <stdio.h>
#include "esp_log.h"
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include <matrixDisplay.h>

#define MAIN_TAG "main"

void app_main()
{
    ESP_LOGI(MAIN_TAG, "Boot successful!!!!");

    display_init();

    // xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}