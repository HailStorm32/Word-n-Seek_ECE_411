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

    SPECIAL_SYMBOLS_START,
    NO_SELECTION = SPECIAL_SYMBOLS_START - 1,
    INCORRECT,
    UNKNOWN,
    CORRECT,
    RIGHT_ARROW,
    LEFT_ARROW,
    SWAPP_ARROWS,
    SPECIAL_SYMBOLS_END,
    
    SPECIAL_SYMBOLS_COUNT = SPECIAL_SYMBOLS_END - SPECIAL_SYMBOLS_START + 1,

    INVALID_SYMBOL = ALPHABET_COUNT + SPECIAL_SYMBOLS_COUNT + 1

} symbols_t;

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
*      Convverts the given character to a symbol type
* 
* Arguments:
*     char character: The character to convert
* 
* Returns:
*      symbols_t: The symbol type of the character
*/
symbols_t charToSymbol(char character);


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
*     Enables the cursor 
* 
* Arguments:
*     None
* 
* Returns:
*      None
*/
void enableCursor(void);


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
*      Returns the current cursor position
*      Just the segment, not the display
* 
* Arguments:
*     None
* 
* Returns:
*      uint8_t: The cursor position
*/
uint8_t getCursorPos(void);


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
*      Just like moveCursor but moves the cursor multiple times
* 
* Arguments:
*     direction_t direction: The direction to move the cursor
*     uint8_t numMoves: The number of times to move the cursor
* 
* Returns:
*      esp_err_t: ESP_OK if the cursor was moved successfully
*/
esp_err_t moveCursorMultiple(direction_t direction, uint8_t numMoves);


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

/*
* Description:
*      Resets the cursor to the starting position (top middle)
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
*      sets the segment to the symbol
* 
* Arguments:
*     symbols_t symbol: The symbol to display
*     display_t display: The display to set the symbol
*     uint8_t charPos: The segment to display the symbol (zero indexed)
* 
* Returns:
*      esp_err_t: ESP_OK if the symbol was set successfully
*/
esp_err_t setSymbol(symbols_t symbol, display_t display, uint8_t charPos);

/*
* Description:
*      Toggle the cursor off
* 
* Arguments:
*     None
* 
* Returns:
*     None
*/
void disableCursor(void);