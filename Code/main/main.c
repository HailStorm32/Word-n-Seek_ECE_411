#include <stdio.h>
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_idf_version.h>
#include <max7219.h>

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif


#define CASCADE_SIZE 4
#define MOSI_PIN 11
#define CS_PIN 10
#define CLK_PIN 12

#define VAL(x) ( (x > 0) ? 2 : 0)


static const uint64_t symbols[] = {

  0xff000001010000ff,
  0xff000003030000ff,
  0xff000006060000ff,
  0xff00000c0c0000ff,
  0xff000018180000ff,
  0xff000030300000ff,
  0xff000060600000ff,
  0xff0000c0c00000ff,
  0xff000080800000ff,
  0xff000000000000ff
    
};
// static const size_t symbols_size = sizeof(symbols) - sizeof(uint64_t) * CASCADE_SIZE;

void task(void *pvParameter)
{
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

    size_t offs = 0;

    max7219_clear(&dev);

    while (1)
    {
        // ESP_LOGI("TEST", "INNN!!!!");
        // for (uint8_t i=0; i<CASCADE_SIZE; i++)
        // max7219_draw_image_8x8(&dev,i*8,(uint8_t *)symbols + i*8 + offs);
        // vTaskDelay(pdMS_TO_TICKS(500));

        //Set the first frame on all segments
        for (uint8_t i=0; i<CASCADE_SIZE; i++)
        {
            max7219_draw_image_8x8(&dev,i*8, symbols + 9);
            // max7219_draw_image_8x8(&dev,i*8, symbols + 8);
            // max7219_draw_image_8x8(&dev,i*8, symbols + 16);
        }

        // for(uint8_t i=0; i<8; i++)
        // {
        //     max7219_draw_image_8x8(&dev,i, symbols + 0);
        //     vTaskDelay(pdMS_TO_TICKS(100));
        // }
        
        for (uint8_t segment=0; segment<CASCADE_SIZE; segment++)
        {
            // To make the animation smoother, we start on the 3rd frame (zero indexed) if not the first segment
            for(uint8_t frame = VAL(segment); frame < sizeof(symbols)/sizeof(symbols[0]) ; frame++)
            {
                max7219_draw_image_8x8(&dev,segment*8,(uint8_t *)symbols + (frame*8));

                // If we are on the second to last frame, we need to draw the first frame on the next segment
                if(frame == 8)
                {
                max7219_draw_image_8x8(&dev,(segment+1)*8, (uint8_t *)symbols + 0);
                }
                // If we are on the last frame, we need to draw the second frame on the next segment
                else if(frame == 9)
                {
                    max7219_draw_image_8x8(&dev,(segment+1)*8, (uint8_t *)symbols + 8);
                }
                
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        

        // if (++offs == symbols_size)
        //     offs = 0;
    }
}

void app_main()
{
    ESP_LOGI("TEST", "Boot successful!!!!");
    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}