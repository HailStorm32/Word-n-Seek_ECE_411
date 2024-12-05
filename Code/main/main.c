#include "esp_log.h"
#include <gpioControl.h>
#include <wifi.h>
#include "api_client.h"
#include <matrixDisplay.h>
#include <wordGuessGame.h>

#define LOG_TAG  "main"

int app_main(void)
{
    initGPIO();
    ESP_LOGI(LOG_TAG, "ESP32 WiFi Station");
    wifi_init_sta();
    api_client_init();
    display_init();  
    ESP_LOGI(LOG_TAG, "Boot successful");
    
    wordGuessGameStart();

    return 0;
}