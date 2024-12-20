#include "esp_log.h"
#include <string.h>
#include <ctype.h>
#include <matrixDisplay.h>
#include <gpioControl.h>
#include <api_client.h>
#include "wordGuessGame.h"


/*-----------------------------------------------------------
Literal Constants
------------------------------------------------------------*/

#define LOG_TAG "WordGuessGame"

#define WORD_SIZE 6 // 5 characters + null terminator

// Map buttons
#define SELECT_BTN      GPIO_BTN_A      //J12 Header ()
#define SELECT_BTN_LED  GPIO_BTN_A_LED
#define GUESS_BTN       GPIO_BTN_B      //J13 Header (BLUE)
#define GUESS_BTN_LED   GPIO_BTN_B_LED  
#define DELETE_BTN      GPIO_BTN_C      //J11 Header (YELLOW)
#define DELETE_BTN_LED  GPIO_BTN_C_LED  
#define EXIT_BTN        GPIO_BTN_D      //J14 Header (RED)
#define EXIT_BTN_LED    GPIO_BTN_D_LED

#define CAROUSEL_START_SEGMENT  1
#define CAROUSEL_MID_SEGMENT    2
#define CAROUSEL_END_SEGMENT    3
#define CAROUSEL_SLIDER_INIT_STRT  ALPHABET_COUNT - 1
#define CAROUSEL_SLIDER_INIT_MID   0
#define CAROUSEL_SLIDER_INIT_END   1

#define BTN_HOLD_DELAY_MS 150 //200

#define MAX_GUESSES 6

#define MAX_BRIGHTNESS 15
#define DEFAULT_BRIGHTNESS 2 // 0 - 15

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

typedef struct 
{
    char apiChar;
    symbols_t equivalentSymbol;
} apiCharMap_t;

/*-----------------------------------------------------------
Memory Constants
------------------------------------------------------------*/

const char carousalCharacters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                                   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
                                   'U', 'V', 'W', 'X', 'Y', 'Z'};

const symbols_t carousalScreenStartState[] = {LEFT_ARROW, Z, A, B, RIGHT_ARROW};
static_assert(CASCADE_SIZE == (sizeof(carousalScreenStartState) / sizeof(symbols_t)), "Invalid carousal start state size");

const symbols_t resultsScreenStartState[] = {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};
static_assert(CASCADE_SIZE == (sizeof(resultsScreenStartState) / sizeof(symbols_t)), "Invalid results start state size");

const apiCharMap_t apiCharMap[] = {
    {'+', CORRECT},
    {'x', SWAPP_ARROWS},
    {'-', INCORRECT}
};

// GOOD \n BYE 
const uint64_t exitScreen[] = {
    0x3c66760606663c00,
    0x3c66666666663c00,
    0x3c66666666663c00,
    0x3e66666666663e00,
    0x0000000000000000,
    0x0000000000000000,
    0x3e66663e66663e00,
    0x1818183c66666600,
    0x7e06063e06067e00,
    0x0000000000000000
};

/*-----------------------------------------------------------
Gobals
------------------------------------------------------------*/

/*-----------------------------------------------------------
Statics
------------------------------------------------------------*/

static uint16_t guessCount;
static uint8_t screenBrightness = DEFAULT_BRIGHTNESS;
static char wordToGuess[WORD_SIZE] = {'-'};
static char guessedWord[WORD_SIZE] = {'-'};
static symbols_t carousalScreenState[CASCADE_SIZE];
static symbols_t resultScreenState[CASCADE_SIZE];
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


/*
* Description:
*      Displays the carousal on the lower display
* 
* Arguments:
*     None
* 
* Returns:
*      esp_err_t: ESP_OK if the carousal was displayed successfully
*/
esp_err_t displayCarousal(void);


/*
* Description:
*      Displays the results on the lower display
* 
* Arguments:
*     None
* 
* Returns:
*      esp_err_t: ESP_OK if the results were displayed successfully
*/
esp_err_t displayResults(void);

/*
* Description:
*      Sets word to all lowercase
* 
* Arguments:
*     char *str: The string to convert to lowercase (needs to be null terminated)
* 
* Returns:
*     None
*/
void toLowercase(char *str);

/*
* Description:
*      Calls the api and validates the guess
*      If the guess is incorrect, the board will be updated
* 
* Arguments:
*     None
* 
* Returns:
*      True if the guess is correct
*      False if the guess is incorrect
*/
bool validateGuess(void);

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

esp_err_t displayCarousal(void)
{
    esp_err_t ret = ESP_OK;

    disableCursor();

    // Display the carousal
    for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
    {
        ret |= setSymbol(carousalScreenState[segment], LOWER_DISPLAY, segment);
    }

    enableCursor();

    return ret;
}

esp_err_t displayResults(void)
{
    esp_err_t ret = ESP_OK;

    // Display the results
    for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
    {
        ret |= setSymbol(resultScreenState[segment], LOWER_DISPLAY, segment);
    }

    return ret;
}

void saveCarousalState(void)
{
    carousalScreenState[1] = charToSymbol(carousalCharacters[carousalSlider.start]);
    carousalScreenState[2] = charToSymbol(carousalCharacters[carousalSlider.mid]);
    carousalScreenState[3] = charToSymbol(carousalCharacters[carousalSlider.end]);
}

void toLowercase(char *str) 
{
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower((unsigned char)str[i]); // Use tolower for each character
    }
}

esp_err_t wordGuessGameReset(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(LOG_TAG, "Reseting word guess game");

    // Reset the carousal
    memcpy(carousalScreenState, carousalScreenStartState, sizeof(carousalScreenState));

    // Reset the carousal slider
    carousalSlider.start = CAROUSEL_SLIDER_INIT_STRT;
    carousalSlider.mid = CAROUSEL_SLIDER_INIT_MID;
    carousalSlider.end = CAROUSEL_SLIDER_INIT_END;

    // Reset the results
    memcpy(resultScreenState, resultsScreenStartState, sizeof(resultScreenState));

    // Reset the board
    disableCursor();
    ret |= resetBoard();
    enableCursor();

    // Reset brightness
    screenBrightness = DEFAULT_BRIGHTNESS;
    setBrightness(screenBrightness);

    // Reset the wordToGuess 
    memset(wordToGuess, '-', sizeof(wordToGuess));
    wordToGuess[WORD_SIZE - 1] = '\0';

    // Retreive the word to guess
    api_get_word(wordToGuess, WORD_SIZE);
    // memcpy(wordToGuess, "HELLO", WORD_SIZE);
    ESP_LOGI(LOG_TAG, "Word to guess: %s", wordToGuess);

    // Reset the guessedWord
    memset(guessedWord, '-', sizeof(guessedWord));
    guessedWord[WORD_SIZE - 1] = '\0';

    // Reset the guess count
    guessCount = 0;

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
            gpio_set_level(SELECT_BTN_LED, 0);
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
                        
                        // Convert the character to a symbol
                        convertedChar = charToSymbol(seclectedChar);

                        // Make sure the character is valid
                        if(convertedChar != INVALID_SYMBOL)
                        {   
                            // Disable the cursor 
                            // This is as a fix for a bug that I dont have time to fix
                            disableCursor();

                            // Set the character on the display
                            setSymbol(convertedChar, UPPER_DISPLAY, cursorPos);
                            
                            // Re-enable the cursor
                            enableCursor();

                            // Save the carousal state
                            saveCarousalState();

                            // Display the result screen
                            displayResults();

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

                        displayCarousal();

                        moveCursor(DOWN);
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
                        getWord(guessedWord, sizeof(guessedWord));

                        ESP_LOGI(LOG_TAG, "Word guessed is: %s", guessedWord);

                        if(validateGuess())
                        {
                            ESP_LOGI(LOG_TAG, "Word guessed is correct");

                            gameState = RESULTS;
                        }
                        else
                        {
                            ESP_LOGI(LOG_TAG, "Word guessed is incorrect");

                            ++guessCount;

                            if(guessCount >= MAX_GUESSES)
                            {
                                // Set the results screen to the word to guess
                                for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
                                {
                                    resultScreenState[segment] = charToSymbol(wordToGuess[segment]);
                                }

                                ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, wordToGuess, WORD_SIZE, ESP_LOG_DEBUG);
                                ESP_LOGI(LOG_TAG, "------");
                                ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, resultScreenState, CASCADE_SIZE, ESP_LOG_DEBUG);

                                // Update the results screen
                                displayResults();

                                gameState = RESULTS;
                            }
                            else
                            {
                                ESP_LOGI(LOG_TAG, "Guesses left: %d", MAX_GUESSES - guessCount);

                                gameState = LETTER_EDIT;
                            }
                        }

                        disableCursor();

                        displayResults();

                        enableCursor();

                        resetCursor();

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
                    case LETTER_SELECTION:
                        // Save the carousal state
                        saveCarousalState();

                        // Display the result screen
                        displayResults();

                        // Reset the cursor
                        resetCursor();

                        gameState = LETTER_EDIT;
                        break;
                    case INIT:
                    case LETTER_EDIT:
                    case RESULTS:
                        displayFullGraphic(exitScreen, sizeof(exitScreen));

                        gpio_set_level(SELECT_BTN_LED, 0);
                        gpio_set_level(GUESS_BTN_LED, 0);
                        gpio_set_level(DELETE_BTN_LED, 0);
                        gpio_set_level(EXIT_BTN_LED, 0);

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
                        // Accommodate long press
                        if(gpio_get_level(GPIO_JOY_LEFT) == 0)
                        {
                            while(gpio_get_level(GPIO_JOY_LEFT) == 0)
                            {
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
                                vTaskDelay(pdMS_TO_TICKS(BTN_HOLD_DELAY_MS));
                            }
                        }
                        // Accommodate single press
                        else
                        {
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
                        }
                        break;
                    case LETTER_EDIT:
                        // Accommodate long press
                        if(gpio_get_level(GPIO_JOY_LEFT) == 0)
                        {
                            while(gpio_get_level(GPIO_JOY_LEFT) == 0)
                            {
                                moveCursor(LEFT);
                                vTaskDelay(pdMS_TO_TICKS(BTN_HOLD_DELAY_MS));
                            }
                        }
                        // Accommodate single press
                        else
                        {
                            moveCursor(LEFT);
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
                * ---------- RIGHT
                *-------------------------------------------------------*/
                case GPIO_JOY_RIGHT:
                    switch(gameState)
                    {
                    case INIT:
                        // Do nothing, no functionality for this button in this state
                        break;
                    case LETTER_SELECTION:
                        // Accommodate long press
                        if(gpio_get_level(GPIO_JOY_RIGHT) == 0)
                        {
                            while(gpio_get_level(GPIO_JOY_RIGHT) == 0)
                            {
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
                                vTaskDelay(pdMS_TO_TICKS(BTN_HOLD_DELAY_MS));
                            }
                        }
                        // Accommodate single press
                        else
                        {
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
                        }
                        break;
                    case LETTER_EDIT:
                        // Accommodate long press
                        if(gpio_get_level(GPIO_JOY_RIGHT) == 0)
                        {
                            while(gpio_get_level(GPIO_JOY_RIGHT) == 0)
                            {
                                moveCursor(RIGHT);
                                vTaskDelay(pdMS_TO_TICKS(BTN_HOLD_DELAY_MS));
                            }
                        }
                        // Accommodate single press
                        else
                        {
                            moveCursor(RIGHT);
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
                * ---------- UP
                *-------------------------------------------------------*/
                case GPIO_JOY_UP:
                    switch(gameState)
                    {
                    case INIT:
                    case LETTER_SELECTION:
                    case LETTER_EDIT:
                    case RESULTS:
                        if(++screenBrightness > MAX_BRIGHTNESS)
                        {
                            screenBrightness = MAX_BRIGHTNESS;
                        }

                        setBrightness(screenBrightness);
                        ESP_LOGI(LOG_TAG, "Brightness: %d", screenBrightness);
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
                    case LETTER_SELECTION:
                    case LETTER_EDIT:
                    case RESULTS:
                        if(--screenBrightness > MAX_BRIGHTNESS)
                        {
                            screenBrightness = 0;
                        }

                        setBrightness(screenBrightness);
                        ESP_LOGI(LOG_TAG, "Brightness: %d", screenBrightness);
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

bool validateGuess(void)
{
    bool isCorrect = true;
    char guessResults[WORD_SIZE];

    memcpy(guessResults, guessedWord, WORD_SIZE);

    toLowercase(guessResults);
    
    api_check_word(guessResults, WORD_SIZE);
    // memcpy(guessResults, "++--x", WORD_SIZE);
    // memcpy(guessResults, "+++++", WORD_SIZE);

    // Set the results
    for(uint8_t segment = 0; segment < CASCADE_SIZE; segment++)
    {   
        // Get the equivalent symbol for the character
        for(uint8_t character = 0; character < WORD_SIZE; character++)
        {
            if(guessResults[segment] == apiCharMap[character].apiChar)
            {
                resultScreenState[segment] = apiCharMap[character].equivalentSymbol;
                break;
            }
        }

        // Flip the is correct flag if at least one character is incorrect
        if( isCorrect && resultScreenState[segment] != CORRECT)
        {
            isCorrect = false;
        }
    }

    return isCorrect;
 }
