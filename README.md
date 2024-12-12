## About

This repo houses the code, board files, CAD files and documents for the ECE 411 course group project.

Word-n-Seek integrates a Wordle-style game into a small-sized arcade style gaming console, using a word retrieved from a Wordle-like API. This approach adds a unique dimension to the gameplay experience compared to using a phone or website.

The gameplay operates similarly to the popular Wordle game. Every day, the device will pull a new five-letter word from the API. The user has six attempts to guess the word, selecting letters with a buttons and confirming choices with selection keys. After each guess, the game marks correctly placed letters. The user continues guessing until either (a) they correctly guess the word or (b) they use all six attempts.

<br>

## Setup

### ESP-IDF Framework
Follow the directions on the [ESP-IDF wiki](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/get-started/index.html#ide) on how to setup the environment

> Note: Use version 5.0 or greater

### "Wordle" API
Create a free account for the [Wordle Game API](https://rapidapi.com/azizbergach/api/wordle-game-api1) to gain an API key

> Note: the free tier only allows 60 requests/day

> Upgrading to the "pro" tier will allow for 5000 requests/day for $5

### Code
Code is under the `Code` directory. Open VS Code(or your preferred editor) in this directory for best results.

Create headers called `hidden.h` in the following directories and format as follows.

#### Components/apiControl/include

    #define  API_KEY  "api_key_string_here"

#### Components/wifiControl/include

    #define  WIFI_SSID  "ssid_string_here"
    #define  WIFI_PASS  "password_string_here"



<br>

## Use

### Buttons
<img src="https://i.imgur.com/7m9EFsD.jpeg" alt="Description" width="400" />

There are four small white directional buttons. Two for LEFT and RIGHT movement of the cursor. Press and hold either to slide the cursor left or right 
As well as two buttons for increasing and decreasing the screen brightness.

Four colored action buttons are to the right of the directional buttons, and are for SELECT, GUESS, DELETE and EXIT. These buttons will selectively light up when you are able to press them.

### Operation
Once the game starts, you will be able to add and delete letters to construct a five letter word (see [Modes](https://github.com/HailStorm32/Word-n-Seek#modes)). 

Once you are satisfied with your word guess, press GUESS to submit your word. The results screen on the bottom row will then update to display the which letters are correct (see [Results](https://github.com/HailStorm32/Word-n-Seek#results)). 

You have six guesses before the game ends. If you reach six guesses without guessing correctly, the results row will be replaced with what the word was supposed to be. The game is now over, press EXIT to exit.

If you guess the word before you use up all six guesses, the results screen will show all correct (see [Results](https://github.com/HailStorm32/Word-n-Seek#results)). The game is now over, press EXIT to exit.


### Modes

#### Startup
Once the device is powered up you will be greeted with the Word n Seek start screen. 

Press SELECT to start the game. Once the game is started, you will be in letter edit mode.

#### Edit Letters
Use the LEFT and RIGHT buttons to move the cursor left or right.

When the cursor is over the desired spot, press SELECT to enter letter select mode to place a letter at that spot, OR press DELETE to delete the selected letter.

#### Letter Selection
In this mode use the LEFT and RIGHT buttons to move the cursor and the letter carousel left and right.

Press SELECT to select the highlighted letter and return to letter edit mode.

Press DELETE to exit letter selection and return to letter edit mode without placing a letter.

### Results
In letter edit mode, the bottom row of the screen will show which letters are correct, incorrect, or are in the wrong place. Below are the possible symbols and what they mean.

#### Unknown
<img src="https://i.imgur.com/aFFMlxq.png" alt="Description" width="200" />

State of letter is unknown
> Before your first guess, the result row will consist of all unknowns ('?')

#### Correct
<img src="https://i.imgur.com/vpmYz89.png" alt="Description" width="200" />

States that the letter is correct and in the right spot
<br>

#### Incorrect
<img src="https://i.imgur.com/UUUwn9d.png" alt="Description" width="200" />

States that the letter is incorrect and not in the word
<br>

#### Swap
<img src="https://i.imgur.com/lf15YIF.png" alt="Description" width="200" />

States that the letter is correct, but in the wrong spot

