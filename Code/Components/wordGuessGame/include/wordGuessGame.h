#pragma once

/*-----------------------------------------------------------
Literal Constants
------------------------------------------------------------*/

/*-----------------------------------------------------------
Memory Constants
------------------------------------------------------------*/

/*-----------------------------------------------------------
Types
------------------------------------------------------------*/

/*-----------------------------------------------------------
Function Prototypes
------------------------------------------------------------*/

/*
* Description:
*      Initializes the word guess game
* 
* Arguments:
*     None
* 
* Returns:
*      esp_err_t: ESP_OK if the game was initialized successfully
*/
esp_err_t wordGuessGameReset(void);


/*
* Description:
*      Starts the word guess game 
*      The game will run until the user exits
* 
* Arguments:
*     None
* 
* Returns:
*      esp_err_t: ESP_OK if the game ran successfully
*/
esp_err_t wordGuessGameStart(void);
