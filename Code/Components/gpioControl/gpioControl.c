#include "esp_log.h"
#include "gpioControl.h"

/*-----------------------------------------------------------
Literal Constants
------------------------------------------------------------*/

//Create a bit masks to select the GPIO pins we want
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<GPIO_BTN_A_LED) | (1ULL<<GPIO_BTN_B_LED) | (1ULL<<GPIO_BTN_C_LED) | (1ULL<<GPIO_BTN_D_LED))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_JOY_LEFT) | (1ULL<<GPIO_JOY_RIGHT) | (1ULL<<GPIO_JOY_UP) | (1ULL<<GPIO_JOY_DOWN) | (1ULL<<GPIO_BTN_A) | (1ULL<<GPIO_BTN_B) | (1ULL<<GPIO_BTN_C) | (1ULL<<GPIO_BTN_D))

#define ESP_INTR_FLAG_DEFAULT 0

#define DEBOUNCE_TIME_MS 200


#define LOG_TAG "gpio_control"


/*-----------------------------------------------------------
Macros
------------------------------------------------------------*/

#define TICKS_TO_MS(ticks) (ticks * portTICK_PERIOD_MS)

/*-----------------------------------------------------------
Types
------------------------------------------------------------*/

/*-----------------------------------------------------------
Gobals
------------------------------------------------------------*/

QueueHandle_t gpioEventQueue = NULL;

/*-----------------------------------------------------------
Statics
------------------------------------------------------------*/

uint64_t s_lastTime;

/*-----------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------*/

static void IRAM_ATTR gpioIsrHandler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    uint64_t curMS = TICKS_TO_MS(xTaskGetTickCountFromISR());

    //To debounce the button, we will only send the event if the last event was more than DEBOUNCE_TIME_MS ago
    if(curMS - s_lastTime > DEBOUNCE_TIME_MS)
    {
        xQueueSendFromISR(gpioEventQueue, &gpio_num, NULL);
    }

    s_lastTime = curMS;
}

/*-----------------------------------------------------------
Functions
------------------------------------------------------------*/

esp_err_t initGPIO()
{
    gpio_config_t ioConf = {}; 
    esp_err_t ret = ESP_OK;

    //initialize the timer
    s_lastTime = 0;

    //disable interrupt
    ioConf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    ioConf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the output pins
    ioConf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    ioConf.pull_down_en = 0;
    //disable pull-up mode
    ioConf.pull_up_en = 0;
    //configure GPIO with the given settings
    ret |= gpio_config(&ioConf);

    //interrupt of rising edge
    ioConf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the input pins
    ioConf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    ioConf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    ioConf.pull_up_en = 1;
    ret |= gpio_config(&ioConf);

    if(ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "Error setting up GPIO pins");
        return ret;
    }
    
    //create a queue to handle gpio event from isr
    gpioEventQueue = xQueueCreate(10, sizeof(uint32_t));

    //install gpio isr service
    ret |= gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //hook up isr handler for each pin
    ret |= gpio_isr_handler_add(GPIO_JOY_LEFT, gpioIsrHandler, (void*) GPIO_JOY_LEFT);
    ret |= gpio_isr_handler_add(GPIO_JOY_RIGHT, gpioIsrHandler, (void*) GPIO_JOY_RIGHT);
    ret |= gpio_isr_handler_add(GPIO_JOY_UP, gpioIsrHandler, (void*) GPIO_JOY_UP);
    ret |= gpio_isr_handler_add(GPIO_JOY_DOWN, gpioIsrHandler, (void*) GPIO_JOY_DOWN);
    ret |= gpio_isr_handler_add(GPIO_BTN_A, gpioIsrHandler, (void*) GPIO_BTN_A);
    ret |= gpio_isr_handler_add(GPIO_BTN_B, gpioIsrHandler, (void*) GPIO_BTN_B);
    ret |= gpio_isr_handler_add(GPIO_BTN_C, gpioIsrHandler, (void*) GPIO_BTN_C);
    ret |= gpio_isr_handler_add(GPIO_BTN_D, gpioIsrHandler, (void*) GPIO_BTN_D);

    if(ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "Error setting up GPIO ISRs");
        return ret;
    }

    return ret;
}