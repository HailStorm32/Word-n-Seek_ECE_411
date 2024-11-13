#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <max7219.h>

#include "matrixDisplay.h"
#include "prvGraphics.h"


/*-----------------------------------------------------------
Literal Constants
------------------------------------------------------------*/

#define MOSI_PIN 11
#define CS_PIN_LWR 10
#define CS_PIN_UPPR 9
#define CLK_PIN 12

#define LEFT_ARROW_SEGMENT  0
#define RIGHT_ARROW_SEGMENT (CASCADE_SIZE - 1)

#define MATRIX_DISP_TAG "matrix_display"


/*-----------------------------------------------------------
Types
------------------------------------------------------------*/

typedef struct
{
    uint8_t CS_PIN;
    max7219_t dev;
} matrixDisplay_t, *matrixDisplayPtr_t;

typedef struct
{
    uint8_t curSegment;
    display_t curDisplay;
    bool isValid;
} cursor_t;

/*-----------------------------------------------------------
Gobals
------------------------------------------------------------*/
uint64_t segmentStates[NUM_DISPLAYS][CASCADE_SIZE];
cursor_t cursor;


/*-----------------------------------------------------------
Statics
------------------------------------------------------------*/
static matrixDisplay_t lowerDisplay;
static matrixDisplay_t upperDisplay;

static matrixDisplayPtr_t displays[NUM_DISPLAYS] = {&lowerDisplay, &upperDisplay};


/*-----------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------*/

/*
* Description:
*      Makes sure the cursor is in a valid position
*      If the cursor is out of bounds, it is corrected
*
*    NOTE: Used when moving the cursor to the lower display
*       where the cursor could be placed on the left and right
*       arrows   
* 
* Arguments:
*    None
* 
* Returns:
*    None
*/
void correctCursorPos(void);

/*
* Description:
*   Displays a graphic accross all displays
*     
*   NOTE:
*      Must contain enough frames to fill all segments
*      Graphic is displayed starting from the far left 
*      of the top display and moves to the right
*
* Arguments:
*      uint64_t *graphic: Pointer to graphic array
*      int size: Size of the graphic array
*
* Returns:
*      esp_err_t: ESP_OK if the graphic was displayed successfully
*/
esp_err_t displayFullGraphic(const uint64_t *graphic, const int size);


/*
* Description:
*      Converts a graphic to a character
* 
* Arguments:
*     uint64_t graphic: The graphic to convert
* 
* Returns:
*      char: The character represented by the graphic
*      '?' if the graphic is not a valid character
*/
char graphicToChar(uint64_t graphic);


/*
* Description:
*   Resets the board to the starting position
*   
* Arguments:
*      None
*
* Returns:
*      esp_err_t: ESP_OK if board was reset successfully
*/
esp_err_t resetBoard(void);

/*-----------------------------------------------------------
Functions
------------------------------------------------------------*/

void clearDisplay(display_t display)
{
    switch (display)
    {
    case LOWER_DISPLAY:
        max7219_clear(&displays[LOWER_DISPLAY]->dev);
        memset(segmentStates[LOWER_DISPLAY], 0, sizeof(segmentStates[LOWER_DISPLAY]));
        break;
    case UPPER_DISPLAY:
        max7219_clear(&displays[UPPER_DISPLAY]->dev);
        memset(segmentStates[UPPER_DISPLAY], 0, sizeof(segmentStates[UPPER_DISPLAY]));
        break;
    case ALL_DISPLAYS:
        max7219_clear(&displays[LOWER_DISPLAY]->dev);
        max7219_clear(&displays[UPPER_DISPLAY]->dev);
        memset(segmentStates, 0, sizeof(segmentStates));
        break;

    default:
        ESP_LOGE(MATRIX_DISP_TAG, "Invalid display");
        break;
    }
}

void correctCursorPos(void)
{   
    // Only do check if cursor is on the lower display
    if(cursor.curDisplay == LOWER_DISPLAY)
    {
        switch (cursor.curSegment)
        {
        case LEFT_ARROW_SEGMENT:
            cursor.curSegment = LEFT_ARROW_SEGMENT + 1;
            break;

        case RIGHT_ARROW_SEGMENT:
            cursor.curSegment = RIGHT_ARROW_SEGMENT - 1;
            break;

        default:
            break;
        }
    }
}

esp_err_t displayFullGraphic(const uint64_t *graphic, const int size)
{
    int frame = 0;
    esp_err_t ret = ESP_OK;

    // Check if pointer is valid
    if(graphic == NULL)
    {
        ESP_LOGE(MATRIX_DISP_TAG, "Invalid graphic pointer");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if there are enough frames to fill all segments
    if(size / sizeof(uint64_t) < NUM_DISPLAYS * CASCADE_SIZE)
    {
        ESP_LOGE(MATRIX_DISP_TAG, "Not enough frames to fill all segments");
        return ESP_ERR_INVALID_SIZE;
    }

    // Display starting from the top display
    for(int display = NUM_DISPLAYS - 1; display >= 0; display--)
    {
        for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
        {   
            // Draw the graphic on the display
            ret |= max7219_draw_image_8x8(&displays[display]->dev, segment*8, &graphic[frame]);

            // Save the segment state
            segmentStates[display][segment] = graphic[frame];

            frame++;
        }
    }

    return ret;
}

esp_err_t display_init(void)
{
    cursor.isValid = false;

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

    // Display the test animation on all modules
    for(size_t frame = 0; frame < sizeof(bootAnimation) / sizeof(uint64_t); frame++)
    {
        for(size_t segment = 0; segment < CASCADE_SIZE; segment++)
        {
            ESP_ERROR_CHECK(max7219_draw_image_8x8(&lowerDisplay.dev, segment*8, &bootAnimation[frame]));
            ESP_ERROR_CHECK(max7219_draw_image_8x8(&upperDisplay.dev, segment*8, &bootAnimation[frame]));
        }
        vTaskDelay(pdMS_TO_TICKS(400));
    }
    vTaskDelay(pdMS_TO_TICKS(400));

    // Clear the displays
    clearDisplay(ALL_DISPLAYS);

    // Diplay the WORD n SEEK! graphic
    ESP_ERROR_CHECK(displayFullGraphic(dispWordNSeek, sizeof(dispWordNSeek)));

    vTaskDelay(pdMS_TO_TICKS(5000));

    // Reset the board
    resetBoard();

    return ESP_OK;
}

char getCharAtCursor(void)
{
    return graphicToChar(segmentStates[cursor.curDisplay][cursor.curSegment]);
}

esp_err_t getWord(char *word, int wordSize)
{
    esp_err_t ret = ESP_OK;
    uint64_t wordGraphicArray[CASCADE_SIZE] = {0};

    // Check if the word array is valid
    if(word == NULL)
    {
        ESP_LOGE(MATRIX_DISP_TAG, "Invalid word array");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if the word array is large enough (including null terminator)
    if(wordSize < CASCADE_SIZE + 1)
    {
        ESP_LOGE(MATRIX_DISP_TAG, "Word array is too small");
        return ESP_ERR_INVALID_SIZE;
    }

    // Get the word in graphic form
    memcpy(wordGraphicArray, segmentStates[UPPER_DISPLAY], CASCADE_SIZE * sizeof(uint64_t));

    // ESP_LOG_BUFFER_HEXDUMP(MATRIX_DISP_TAG, wordGraphicArray, wordSize * sizeof(uint64_t), ESP_LOG_DEBUG);
    // ESP_LOG_BUFFER_HEXDUMP(MATRIX_DISP_TAG, segmentStates[UPPER_DISPLAY], CASCADE_SIZE * sizeof(uint64_t), ESP_LOG_DEBUG);

    // Convert the graphic to characters
    for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
    {
        word[segment] = graphicToChar(wordGraphicArray[segment]);
    }

    // Add null terminator
    word[CASCADE_SIZE + 1] = '\0';

    return ret;
}

char graphicToChar(uint64_t graphic)
{   
    // Check if graphic is a letter
    for(uint8_t index = 0; index < ALPHABET_COUNT; index++)
    {   
        // Also account for the inverted graphic (aka the cursor)
        if((graphic == graphicsLetter[index]) || (graphic == ~graphicsLetter[index]))
        {
            return 'A' + index;
        }
    }

    // Check if cursor is on a special character
    switch(graphic)
    {
    case GRAPHIC_NO_SELECTION:
    case ~GRAPHIC_NO_SELECTION:
        return '-';
        break;
    
    default:
        break;
    }

    ESP_LOGD(MATRIX_DISP_TAG, "Invalid graphic: %llx", graphic);

    return '?';
}

esp_err_t moveCursor(direction_t direction)
{
    esp_err_t ret = ESP_OK;
    uint64_t * currentSegmentState;

    // Get pointer to the current segment state
    currentSegmentState = &segmentStates[cursor.curDisplay][cursor.curSegment];

    // Take the cursor graphic out of the current segment
    *currentSegmentState = ~*currentSegmentState;

    // Clear the current cursor
    ret |= max7219_draw_image_8x8(&displays[cursor.curDisplay]->dev, cursor.curSegment * 8, currentSegmentState);

    // Move the cursor
    switch (direction)
    {
    case UP:
        if(cursor.curDisplay == LOWER_DISPLAY)
        {
            cursor.curDisplay = UPPER_DISPLAY;
        }
        else
        {
            cursor.curDisplay = LOWER_DISPLAY;

            correctCursorPos();
        }
        break;
    case DOWN:
        if(cursor.curDisplay == UPPER_DISPLAY)
        {
            cursor.curDisplay = LOWER_DISPLAY;

            correctCursorPos();
        }
        else
        {
            cursor.curDisplay = UPPER_DISPLAY;
        }
        break;
    case LEFT:
        if(cursor.curSegment > 0)
        {
            cursor.curSegment--;
        }
        else
        {
            cursor.curSegment = CASCADE_SIZE - 1;
        }
        correctCursorPos();
        break;
    case RIGHT:
        if(cursor.curSegment < CASCADE_SIZE - 1)
        {
            cursor.curSegment++;
        }
        else
        {
            cursor.curSegment = 0;
        }
        correctCursorPos();
        break;
    
    default:
        ESP_LOGE(MATRIX_DISP_TAG, "Invalid cursor direction");
        ret = ESP_ERR_NOT_SUPPORTED;
        break;
    }

    // Get pointer to the new current segment state
    currentSegmentState = &segmentStates[cursor.curDisplay][cursor.curSegment];

    // Create the new cursor graphic
    *currentSegmentState = ~*currentSegmentState;

    // Display the cursor
    ret |= max7219_draw_image_8x8(&displays[cursor.curDisplay]->dev, cursor.curSegment * 8, currentSegmentState);

    return ret;
}

esp_err_t resetBoard(void)
{
    esp_err_t ret = ESP_OK;

    clearDisplay(ALL_DISPLAYS);

    // Display the empty board
    ret |= displayFullGraphic(dispEmptyBoard, sizeof(dispEmptyBoard));

    // Reset the cursor
    ret |= resetCursor();
    
    return ret;
}

esp_err_t resetCursor(void)
{
    esp_err_t ret = ESP_OK;

    if(cursor.isValid)
    {
        // Clear the current cursor
        segmentStates[cursor.curDisplay][cursor.curSegment] = ~segmentStates[cursor.curDisplay][cursor.curSegment];
        ret |= max7219_draw_image_8x8(&displays[cursor.curDisplay]->dev, cursor.curSegment * 8, &segmentStates[cursor.curDisplay][cursor.curSegment]);
    }

    // Reset the cursor
    cursor.curDisplay = LOWER_DISPLAY;
    cursor.curSegment = 2;

    // Display the new cursor
    segmentStates[cursor.curDisplay][cursor.curSegment] = ~segmentStates[cursor.curDisplay][cursor.curSegment];
    ret |= max7219_draw_image_8x8(&displays[cursor.curDisplay]->dev, cursor.curSegment * 8, &segmentStates[cursor.curDisplay][cursor.curSegment]);

    cursor.isValid = true;
    
    return ret;
}