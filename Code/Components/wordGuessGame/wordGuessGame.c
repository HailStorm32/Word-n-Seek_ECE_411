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

/*-----------------------------------------------------------
Gobals
------------------------------------------------------------*/

/*-----------------------------------------------------------
Statics
------------------------------------------------------------*/

static char wordGuess[WORD_SIZE] = {'-'};
static wordGuessGameStates_t gameState = INIT;

/*-----------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------*/

/*-----------------------------------------------------------
Functions
------------------------------------------------------------*/
esp_err_t wordGuessGameReset(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(LOG_TAG, "Reseting word guess game");

    // Reset the board
    ret |= resetBoard();

    // Reset the wordGuess 
    memset(wordGuess, '-', sizeof(wordGuess));

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

    ESP_LOGI(LOG_TAG, "Starting word guess game");

    // setCharacter(E, 0);
    // setCharacter(C, 1);
    // setCharacter(E, 2);

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
            gpio_set_level(GUESS_BTN_LED, 0);
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
            printf("GPIO[%"PRIu32"] intr\n", ioNum);

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

                        // Update the display
                        

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
                        /* code */
                        break;
                    case LETTER_EDIT:
                        /* code */
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
                        moveCursor(LEFT);
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
                        moveCursor(RIGHT);
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
