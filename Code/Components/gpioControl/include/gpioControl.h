#pragma once

#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

//Define the GPIO pins we want to use
#define GPIO_JOY_LEFT   3
#define GPIO_JOY_RIGHT  6
#define GPIO_JOY_UP     5
#define GPIO_JOY_DOWN   4
#define GPIO_BTN_A      13
#define GPIO_BTN_B      14
#define GPIO_BTN_C      7
#define GPIO_BTN_D      15

#define GPIO_BTN_A_LED 16 
#define GPIO_BTN_B_LED 17
#define GPIO_BTN_C_LED 18
#define GPIO_BTN_D_LED 8

extern QueueHandle_t gpioEventQueue;

/*
* Description:
*      sets up the GPIO pins
*
* Arguments:
*      None
*
* Returns:
*      true -- if the GPIO pins were successfully set up
*      false -- if the GPIO pins were not successfully set up
*/
esp_err_t initGPIO();