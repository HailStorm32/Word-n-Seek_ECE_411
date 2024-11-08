#include "esp_log.h"
#include <gpioControl.h>
#include "esp_log.h"
#include <wifi.h>


#define LOG_TAG "MAIN"

int app_main(void)
{
    uint32_t ioNum;

    initGPIO();
    ESP_LOGI(LOG_TAG, "ESP32 WiFi Station");
    wifi_init_sta();

    ESP_LOGI(LOG_TAG, "Boot successful");

    while(1)
    {
        if(xQueueReceive(gpioEventQueue, &ioNum, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr\n", ioNum);

            if(ioNum == GPIO_BTN_A || ioNum == GPIO_BTN_B || ioNum == GPIO_BTN_C || ioNum == GPIO_BTN_D) {
                switch (ioNum)
                {
                case GPIO_BTN_A:
                    gpio_set_level(GPIO_BTN_A_LED, 1);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    gpio_set_level(GPIO_BTN_A_LED, 0);
                    break;
                case GPIO_BTN_B:
                    gpio_set_level(GPIO_BTN_B_LED, 1);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    gpio_set_level(GPIO_BTN_B_LED, 0);
                    break;
                case GPIO_BTN_C:
                    gpio_set_level(GPIO_BTN_C_LED, 1);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    gpio_set_level(GPIO_BTN_C_LED, 0);
                    break;
                case GPIO_BTN_D:
                    gpio_set_level(GPIO_BTN_D_LED, 1);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    gpio_set_level(GPIO_BTN_D_LED, 0);
                    break;
                
                default:
                    break;
                }
            }
            else if(ioNum == GPIO_JOY_LEFT || ioNum == GPIO_JOY_RIGHT || ioNum == GPIO_JOY_UP || ioNum == GPIO_JOY_DOWN) {
                printf("Joystick press\n");
            }
        }
    }

    return 0;
}