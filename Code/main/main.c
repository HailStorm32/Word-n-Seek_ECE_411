#include "esp_log.h"
#include <gpioControl.h>
#include <matrixDisplay.h>
#include <wordGuessGame.h>

#define LOG_TAG  "main"

int app_main(void)
{
    initGPIO();
    display_init();

    ESP_LOGI(LOG_TAG, "Boot successful");

    wordGuessGameStart();

    return 0;
}