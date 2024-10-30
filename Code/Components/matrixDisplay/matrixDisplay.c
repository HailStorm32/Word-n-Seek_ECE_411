#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <max7219.h>

#define CASCADE_SIZE 4

#define MOSI_PIN 11
#define CS_PIN 10
#define CLK_PIN 12

#define MATRIX_DISP_TAG "matrix_display"

static const uint64_t bootAnimation[] = {
    0xffffffffffffffff,
    0xfff9c1bdffffdbff,
    0x00063e4200002400
};

esp_err_t display_init(void)
{
    ESP_LOGI(MATRIX_DISP_TAG, "Initializing display");

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

    // Configure device
    max7219_t dev = {
        .cascade_size = CASCADE_SIZE,
        .digits = 0,
        .mirrored = true
    };
    ESP_ERROR_CHECK(max7219_init_desc(&dev, SPI2_HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));

    // Clear the display
    max7219_clear(&dev);

    // Display the boot animation on all modules
    for(size_t frame = 0; frame < sizeof(bootAnimation) / sizeof(uint64_t); frame++)
    {
        for(size_t segment = 0; segment < CASCADE_SIZE; segment++)
        {
            ESP_ERROR_CHECK(max7219_draw_image_8x8(&dev, segment*8, &bootAnimation[frame]));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    vTaskDelay(pdMS_TO_TICKS(700));

    // Clear the display
    max7219_clear(&dev);

    return ESP_OK;
}