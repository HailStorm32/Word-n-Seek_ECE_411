#include "esp_log.h"
#include <string.h>
#include <matrixDisplay.h>
#include <gpioControl.h>
#include "wordGuessGame.h"


/*-----------------------------------------------------------
Literal Constants
------------------------------------------------------------*/

#define LOG_TAG "WordGuessGame"

#define WORD_SIZE 6 // 5 characters + null terminator

// Map buttons
#define SELECT_BTN      GPIO_BTN_A
#define SELECT_BTN_LED  GPIO_BTN_A_LED
#define GUESS_BTN       GPIO_BTN_B
#define GUESS_BTN_LED   GPIO_BTN_B_LED
#define DELETE_BTN      GPIO_BTN_C
#define DELETE_BTN_LED  GPIO_BTN_C_LED
#define EXIT_BTN        GPIO_BTN_D
#define EXIT_BTN_LED    GPIO_BTN_D_LED

#define CAROUSEL_START_SEGMENT  1
#define CAROUSEL_MID_SEGMENT    2
#define CAROUSEL_END_SEGMENT    3
#define CAROUSEL_SLIDER_INIT_STRT  ALPHABET_COUNT - 1
#define CAROUSEL_SLIDER_INIT_MID   0
#define CAROUSEL_SLIDER_INIT_END   1

/*-----------------------------------------------------------
Memory Constants
------------------------------------------------------------*/

const char carousalCharacters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                                   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
                                   'U', 'V', 'W', 'X', 'Y', 'Z'};

/*-----------------------------------------------------------
Types
------------------------------------------------------------*/

typedef enum 
{
    INIT,
    LETTER_SELECTION,
    LETTER_EDIT,
    RESULTS
} wordGuessGameStates_t;

typedef struct
{
    uint8_t start;
    uint8_t mid;
    uint8_t end;
}carousalSliderPos_t;

/*-----------------------------------------------------------
Gobals
------------------------------------------------------------*/

/*-----------------------------------------------------------
Statics
------------------------------------------------------------*/

static char wordGuess[WORD_SIZE] = {'-'};
static wordGuessGameStates_t gameState = INIT;
carousalSliderPos_t carousalSlider = {CAROUSEL_SLIDER_INIT_STRT, CAROUSEL_SLIDER_INIT_MID, CAROUSEL_SLIDER_INIT_END};

/*-----------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------*/

/*
* Description:
*      Cycles the carousal in the given direction
* 
* Arguments:
*     direction_t direction: The direction to cycle the carousal
* 
* Returns:
*      esp_err_t: ESP_OK if the carousal was cycled successfully
*/
esp_err_t cycleCarousal(direction_t direction);



/*-----------------------------------------------------------
Functions
------------------------------------------------------------*/

esp_err_t cycleCarousal(direction_t direcion)
{
    esp_err_t ret = ESP_OK;

    switch(direcion)
    {
    case LEFT:
        // Move the carousal to the left
        carousalSlider.start = (carousalSlider.start - 1 + ALPHABET_COUNT) % ALPHABET_COUNT;
        carousalSlider.mid = (carousalSlider.mid - 1 + ALPHABET_COUNT) % ALPHABET_COUNT;
        carousalSlider.end = (carousalSlider.end - 1 + ALPHABET_COUNT) % ALPHABET_COUNT;
        break;
    case RIGHT:
        // Move the carousal to the right
        carousalSlider.start = (carousalSlider.start + 1) % ALPHABET_COUNT;
        carousalSlider.mid = (carousalSlider.mid + 1) % ALPHABET_COUNT;
        carousalSlider.end = (carousalSlider.end + 1) % ALPHABET_COUNT;
        break;
    default:
        ESP_LOGE(LOG_TAG, "Invalid direction given");
        break;
    }

    // Display the carousal
    ret |= setSymbol(charToSymbol(carousalCharacters[carousalSlider.start]), LOWER_DISPLAY, CAROUSEL_START_SEGMENT);
    ret |= setSymbol(charToSymbol(carousalCharacters[carousalSlider.mid]), LOWER_DISPLAY, CAROUSEL_MID_SEGMENT);
    ret |= setSymbol(charToSymbol(carousalCharacters[carousalSlider.end]), LOWER_DISPLAY, CAROUSEL_END_SEGMENT);

    return ret;
}

esp_err_t wordGuessGameReset(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(LOG_TAG, "Reseting word guess game");

    // Reset the carousal slider
    carousalSlider.start = CAROUSEL_SLIDER_INIT_STRT;
    carousalSlider.mid = CAROUSEL_SLIDER_INIT_MID;
    carousalSlider.end = CAROUSEL_SLIDER_INIT_END;

    // Reset the board
    ret |= resetBoard();

    // Reset the wordGuess 
    memset(wordGuess, '-', sizeof(wordGuess));
    wordGuess[WORD_SIZE - 1] = '\0';

    // Reset the game state
    gameState = INIT;

    return ESP_OK;
}

esp_err_t wordGuessGameStart(void)
{   
    bool isRunning = true;
    uint32_t ioNum;
    char seclectedChar;
    uint8_t cursorPos = 2;
    uint8_t cursorPos2 = 2; // TODO: Find a better name
    symbols_t convertedChar;

    ESP_LOGI(LOG_TAG, "Starting word guess game");

    while(isRunning)
    {

        // Set option button LEDs based on game state
        switch(gameState)
        {
        case INIT:
            gpio_set_level(SELECT_BTN_LED, 1);
            gpio_set_level(GUESS_BTN_LED, 0);
            gpio_set_level(DELETE_BTN_LED, 0);
            gpio_set_level(EXIT_BTN_LED, 1);
            break;
        case LETTER_SELECTION:
            gpio_set_level(SELECT_BTN_LED, 1);
            gpio_set_level(GUESS_BTN_LED, 0);
            gpio_set_level(DELETE_BTN_LED, 0);
            gpio_set_level(EXIT_BTN_LED, 1);
            break;
        case LETTER_EDIT:
            gpio_set_level(SELECT_BTN_LED, 1);
            gpio_set_level(GUESS_BTN_LED, 1);
            gpio_set_level(DELETE_BTN_LED, 1);
            gpio_set_level(EXIT_BTN_LED, 1);
            break;
        case RESULTS:
            gpio_set_level(SELECT_BTN_LED, 1);
            gpio_set_level(GUESS_BTN_LED, 0);
            gpio_set_level(DELETE_BTN_LED, 0);
            gpio_set_level(EXIT_BTN_LED, 1);
            break;
        default:
            ESP_LOGE(LOG_TAG, "Given invalid game state");
            break;
        }


        if(xQueueReceive(gpioEventQueue, &ioNum, portMAX_DELAY)) 
        {
            // printf("GPIO[%"PRIu32"] intr\n", ioNum);

            if(ioNum == SELECT_BTN || ioNum == GUESS_BTN || ioNum == DELETE_BTN || ioNum == EXIT_BTN) 
            {
                switch (ioNum)
                {
                /*-------------------------------------------------------
                * ---------- SELECT_BTN
                *-------------------------------------------------------*/
                case SELECT_BTN:
                    switch(gameState)
                    {
                    case INIT:
                        wordGuessGameReset();
                        gameState = LETTER_EDIT;
                        break;
                    case LETTER_SELECTION:
                        // Get the selected character
                        seclectedChar = getCharAtCursor();

                        // Set the selected character
                        wordGuess[cursorPos] = seclectedChar;
                        
                        // Convert the character to a symbol
                        convertedChar = charToSymbol(seclectedChar);

                        // Make sure the character is valid
                        if(convertedChar != INVALID_SYMBOL)
                        {   
                            // Disable the cursor 
                            // This is as a fix for a bug that I dont have time to fix
                            toggleCursor();

                            // Set the character on the display
                            setSymbol(convertedChar, UPPER_DISPLAY, cursorPos);
                            
                            // Re-enable the cursor
                            toggleCursor();

                            // Reset the cursor
                            resetCursor();

                            // Set the next game state 
                            gameState = LETTER_EDIT;
                        }
                        else
                        {
                            ESP_LOGE(LOG_TAG, "Invalid character selected");
                        }

                        break;
                    case LETTER_EDIT:
                        // Set the next game state
                        gameState = LETTER_SELECTION;

                        // Get cursor position
                        cursorPos = getCursorPos();

                        // Get the cursor to the center
                        moveCursorMultiple(RIGHT, (2 - cursorPos + CASCADE_SIZE) % CASCADE_SIZE);

                        moveCursor(DOWN);
                        break;
                    case RESULTS:
                        /* code */
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;

                /*-------------------------------------------------------
                * ---------- GUESS_BTN
                *-------------------------------------------------------*/
                case GUESS_BTN:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_EDIT:
                        // TODO: Add guess call
                        ESP_LOGI(LOG_TAG, "Guessing word: %s", wordGuess);
                        gameState = RESULTS;
                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;
                
                /*-------------------------------------------------------
                * ---------- DELETE_BTN
                *-------------------------------------------------------*/
                case DELETE_BTN:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Do nothing, no functionality for this button in this state
                        //TODO: maybe delete all characters in the word of the selected character
                        break;
                    case LETTER_EDIT:
                        // Get cursor position
                        cursorPos = getCursorPos();

                        // Set the character to a dash
                        wordGuess[cursorPos] = '-';

                        // Convert the character to a symbol
                        convertedChar = charToSymbol('-');

                        // Set the character on the display only if the symbol is valid
                        if(convertedChar != INVALID_SYMBOL)
                        {
                            setSymbol(convertedChar, UPPER_DISPLAY, cursorPos);
                        }
                        else
                        {
                            ESP_LOGE(LOG_TAG, "Invalid character selected");
                        }

                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;

                /*-------------------------------------------------------
                * ---------- EXIT_BTN
                *-------------------------------------------------------*/
                case EXIT_BTN:
                    switch(gameState)
                    {
                    case INIT:
                        isRunning = false;
                        break;
                    case LETTER_SELECTION:
                        isRunning = false;
                        break;
                    case LETTER_EDIT:
                        isRunning = false;
                        break;
                    case RESULTS:
                        isRunning = false;
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;
                    ESP_LOGI(LOG_TAG, "Exiting word guess game");
                
                default:
                    break;
                }
            }
            else if(ioNum == GPIO_JOY_LEFT || ioNum == GPIO_JOY_RIGHT || ioNum == GPIO_JOY_UP || ioNum == GPIO_JOY_DOWN) 
            {
                switch (ioNum)
                {
                /*-------------------------------------------------------
                * ---------- LEFT
                *-------------------------------------------------------*/
                case GPIO_JOY_LEFT:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Get the cursor position
                        cursorPos2 = getCursorPos();

                        if(cursorPos2 == CAROUSEL_START_SEGMENT)
                        {
                            // Move the carousal to the left
                            cycleCarousal(LEFT);
                        }
                        else
                        {
                            // Move the cursor to the left
                            moveCursor(LEFT);
                        }
                        break;
                    case LETTER_EDIT:
                        moveCursor(LEFT);
                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;

                /*-------------------------------------------------------
                * ---------- RIGHT
                *-------------------------------------------------------*/
                case GPIO_JOY_RIGHT:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Get the cursor position
                        cursorPos2 = getCursorPos();

                        if(cursorPos2 == CAROUSEL_END_SEGMENT)
                        {
                            // Move the carousal to the right
                            cycleCarousal(RIGHT);
                        }
                        else
                        {
                            // Move the cursor to the right
                            moveCursor(RIGHT);
                        }
                        break;
                    case LETTER_EDIT:
                        moveCursor(RIGHT);
                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;

                /*-------------------------------------------------------
                * ---------- UP
                *-------------------------------------------------------*/
                case GPIO_JOY_UP:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_EDIT:
                        /* code */
                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    break;

                /*-------------------------------------------------------
                * ---------- DOWN
                *-------------------------------------------------------*/
                case GPIO_JOY_DOWN:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_EDIT:
                        /* code */
                        break;
                    case RESULTS:
                        // Do nothing, no functionality for this button in this state
                        break;
                    default:
                        ESP_LOGE(LOG_TAG, "Given invalid game state");
                        break;
                    }
                    // moveCursor(DOWN);
                    break;
                
                default:
                    break;
                }
            }
            // ESP_LOGD(LOG_TAG, "Character is: %c", getCharAtCursor());
            // getWord(word, sizeof(word));
            // ESP_LOGD(LOG_TAG, "Word is: %s", word);

        }
    }

    return ESP_OK;
}
