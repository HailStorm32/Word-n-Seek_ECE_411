#include "esp_log.h"
#include <gpioControl.h>
#include <wifi.h>
#include "api_client.h"
#include <matrixDisplay.h>

#define LOG_TAG  "main"

int app_main(void)
{
    uint32_t ioNum;
    char word[6] = {0};
    char* word2;
    initGPIO();
    ESP_LOGI(LOG_TAG, "ESP32 WiFi Station");
    wifi_init_sta();
    api_client_init();
    display_init();
    
    //Fetching word of the day from API
    word2 = malloc(sizeof(char)* 6);
    ESP_LOGI(LOG_TAG, "Fetching word from API");
    api_get_word(word2, 6);

    if (word2) {
        ESP_LOGI(LOG_TAG, "Word retrieved: %s", word2);
        // Display the word
    } else {
        ESP_LOGE(LOG_TAG, "Failed to get word from API");
    }
    ESP_LOGI(LOG_TAG, "Entering api_check_word");
    api_check_word(word2, 6);
    ESP_LOGI(LOG_TAG, "Post api_check_word");
    if (word2) {
        ESP_LOGI(LOG_TAG, "Word Checked: %s", word2);
        // Display the word
    } else {
        ESP_LOGE(LOG_TAG, "Failed to get word from API");
    }
    
    ESP_LOGI(LOG_TAG, "Boot successful");

    setCharacter(E, 0);
    setCharacter(C, 1);
    setCharacter(E, 2);

    while(1)
    {
        if(xQueueReceive(gpioEventQueue, &ioNum, portMAX_DELAY)) 
        {
            printf("GPIO[%"PRIu32"] intr\n", ioNum);

            if(ioNum == GPIO_BTN_A || ioNum == GPIO_BTN_B || ioNum == GPIO_BTN_C || ioNum == GPIO_BTN_D) 
            {
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
            else if(ioNum == GPIO_JOY_LEFT || ioNum == GPIO_JOY_RIGHT || ioNum == GPIO_JOY_UP || ioNum == GPIO_JOY_DOWN) 
            {
                switch (ioNum)
                {
                case GPIO_JOY_LEFT:
                    moveCursor(LEFT);
                    break;
                case GPIO_JOY_RIGHT:
                    moveCursor(RIGHT);
                    break;
                case GPIO_JOY_UP:
                    moveCursor(UP);
                    break;
                case GPIO_JOY_DOWN:
                    moveCursor(DOWN);
                    break;
                
                default:
                    break;
                }
            }
            ESP_LOGD(LOG_TAG, "Character is: %c", getCharAtCursor());
            getWord(word, sizeof(word));
            ESP_LOGD(LOG_TAG, "Word is: %s", word);

        }
    }

    return 0;
}