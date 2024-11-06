#pragma once

#include <esp_err.h>

#define CASCADE_SIZE 5  // Number of cascaded MAX7219 modules in a display
#define NUM_DISPLAYS 2  // Number of displays in the system

typedef enum
{
    LOWER_DISPLAY = 0,
    UPPER_DISPLAY,
    
    ALL_DISPLAYS
} display_t;

typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction_t;

extern uint64_t segmentStates[NUM_DISPLAYS][CASCADE_SIZE];


/*
* Description:
*      Clears selected display
*
* Arguments:
*      display_t display: The display to clear
*
* Returns:
*      None
*/
void clearDisplay(display_t display);


/*
* Description:
*      Initializes the display and sets the display to the starting position
* 
* Arguments:
*      None
* 
* Returns:
*      esp_err_t: ESP_OK if the display was initialized successfully
*/
esp_err_t display_init(void);


/*
* Description:
*      Moves the cursor in the specified direction
* 
* Arguments:
*     direction_t direction: The direction to move the cursor
* 
* Returns:
*      esp_err_t: ESP_OK if the cursor was moved successfully
*/
esp_err_t moveCursor(direction_t direction);


/*
* Description:
*      Resets the cursor to the starting position (bottom middle)
* 
* Arguments:
*     None
* 
* Returns:
*      esp_err_t: ESP_OK if the cursor was reset successfully
*/
esp_err_t resetCursor(void);