#ifndef API_CLIENT_H
#define API_CLIENT_H

#include "esp_err.h"

// Function prototype for sending a POST request to an API endpoint
esp_err_t api_get_word(char* word, int word_size);
esp_err_t api_check_word(char* guess, int guess_size);
esp_err_t api_client_init(void);

#endif // API_CLIENT_H
