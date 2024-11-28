#ifndef API_CLIENT_H
#define API_CLIENT_H

#include "esp_err.h"

// Function prototype for sending a POST request to an API endpoint
esp_err_t api_send_post_request(const char *url, const char *post_data);

esp_err_t api_client_init(void);
char* api_get_word(void);


#endif // API_CLIENT_H
