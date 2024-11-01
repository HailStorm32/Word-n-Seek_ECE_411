#include <esp_err.h>

typedef enum
{
    LOWER_DISPLAY = 0,
    UPPER_DISPLAY,
    
    ALL_DISPLAYS
} display_t;

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