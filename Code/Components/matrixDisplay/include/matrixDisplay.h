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

typedef enum
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    ALPHABET_COUNT,

    SPECIAL_CHARACTERS_START,
    NO_SELECTION = SPECIAL_CHARACTERS_START,
    INCORRECT,
    SPECIAL_CHARACTERS_END,
    
    SPECIAL_CHARACTERS_COUNT = SPECIAL_CHARACTERS_END - SPECIAL_CHARACTERS_START

} characters_t;

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
*      Returns the character at the cursor
* 
* Arguments:
*     None
* 
* Returns:
*      char: The character at the cursor
*      '?' if the cursor is not on valid a character
*/
char getCharAtCursor(void);


/*
* Description:
*      returns a null terminated string of characters from the top display
*       aka the word
* 
* Arguments:
*     char *word: The array to store the word (must account for null terminator)
*     int wordSize: The size of the array
* 
* Returns:
*      esp_err_t: ESP_OK if the word was retrieved successfully
*/
esp_err_t getWord(char *word, int wordSize);


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

/*
* Description:
*      sets the segment to the character
* 
* Arguments:
*     characters_t character: The character to display
*     uint8_t charPos: The segment to display the character (zero indexed)
* 
* Returns:
*      esp_err_t: ESP_OK if the character was set successfully
*/
esp_err_t setCharacter(characters_t character, uint8_t charPos);