#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <max7219.h>

#include "matrixDisplay.h"
#include "prvGraphics.h"

#define CASCADE_SIZE 5  // Number of cascaded MAX7219 modules in a display
#define NUM_DISPLAYS 2  // Number of displays in the system

#define MOSI_PIN 11
#define CS_PIN_LWR 10
#define CS_PIN_UPPR 9
#define CLK_PIN 12

#define MATRIX_DISP_TAG "matrix_display"

typedef struct
{
    uint8_t CS_PIN;
    max7219_t dev;
} matrixDisplay_t, *matrixDisplayPtr_t;

static matrixDisplay_t lowerDisplay;
static matrixDisplay_t upperDisplay;

static matrixDisplayPtr_t displays[NUM_DISPLAYS] = {&lowerDisplay, &upperDisplay};

void clearDisplay(display_t display)
{
switch (display)
{
case LOWER_DISPLAY:
    max7219_clear(&displays[LOWER_DISPLAY]->dev);
    break;
case UPPER_DISPLAY:
    max7219_clear(&displays[UPPER_DISPLAY]->dev);
    break;
case ALL_DISPLAYS:
    max7219_clear(&displays[LOWER_DISPLAY]->dev);
    max7219_clear(&displays[UPPER_DISPLAY]->dev);
    break;

default:
    ESP_LOGE(MATRIX_DISP_TAG, "Invalid display");
    break;
}
}

esp_err_t display_init(void)
{
    // Set the CS pins for the displays
    displays[LOWER_DISPLAY]->CS_PIN = CS_PIN_LWR;
    displays[UPPER_DISPLAY]->CS_PIN = CS_PIN_UPPR;

    // Configure SPI bus
    spi_bus_config_t cfg = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &cfg, SPI_DMA_CH_AUTO));


    // Initialize the displays
    for(uint8_t display = 0; display < NUM_DISPLAYS; display++)
    {
        ESP_LOGI(MATRIX_DISP_TAG, "Initializing display %d", display);
        max7219_t *dev = &displays[display]->dev;

        // Configure display
        dev->cascade_size = CASCADE_SIZE;
        dev->digits = 0;
        dev->mirrored = true;

        ESP_ERROR_CHECK(max7219_init_desc(dev, SPI2_HOST, MAX7219_MAX_CLOCK_SPEED_HZ, displays[display]->CS_PIN));
        ESP_ERROR_CHECK(max7219_init(dev));
    }

    // Clear the displays
    clearDisplay(ALL_DISPLAYS);

    // Display the boot animation on all modules
    for(size_t frame = 0; frame < sizeof(bootAnimation) / sizeof(uint64_t); frame++)
    {
        for(size_t segment = 0; segment < CASCADE_SIZE; segment++)
    {
            ESP_ERROR_CHECK(max7219_draw_image_8x8(&lowerDisplay.dev, segment*8, &bootAnimation[frame]));
            ESP_ERROR_CHECK(max7219_draw_image_8x8(&upperDisplay.dev, segment*8, &bootAnimation[frame]));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    vTaskDelay(pdMS_TO_TICKS(700));

    // Clear the displays
    clearDisplay(ALL_DISPLAYS);

    return ESP_OK;
}