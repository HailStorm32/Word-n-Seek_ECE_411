#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "api_client.h" 
#include "hidden.h"     // Contains API_KEY

const char *root_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF\n"
    "ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj\n"
    "b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x\n"
    "OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1\n"
    "dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/\n"
    "BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW\n"
    "gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH\n"
    "MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH\n"
    "MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy\n"
    "MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0\n"
    "LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW\n"
    "MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma\n"
    "eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK\n"
    "bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN\n"
    "0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U\n"
    "akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==\n"
    "-----END CERTIFICATE-----";

#define WORD_SIZE 6
#define BUFFER_SIZE 1024
#define GET_WORD_URL "https://wordle-game-api1.p.rapidapi.com/word"
#define CHECK_WORD_URL "https://wordle-game-api1.p.rapidapi.com/guess"

typedef struct {
    char *buffer;
    int buffer_size;
    int data_length;
} response_data_t;

typedef enum {
    HTTP_SUCCESS = 0,
    HTTP_ERROR_OPEN_CONNECTION,
    HTTP_ERROR_WRITE_FAILED,
    HTTP_ERROR_FETCH_HEADERS_FAILED,
    HTTP_ERROR_READ_RESPONSE_FAILED
} http_error_t;

static const char *TAG = "api_client";
static esp_http_client_handle_t client;
static esp_http_client_config_t config;
static response_data_t response;
static void extract_word(const char *json_string, size_t json_size, char *word_buffer, size_t buffer_size);
static void extract_result(const char *json_string, size_t json_size, char *word_buffer, size_t buffer_size);

/**
 * @brief Makes an HTTP POST request to the specified URL with the given post data.
 *
 * @param url The URL to send the POST request to.
 * @param post_data The data to include in the POST request body.
 * @param output_buffer Buffer to store the response.
 * @param buffer_size Size of the output_buffer.
 * @return http_error_t Returns an error code indicating the success or failure of the request.
 */
static http_error_t make_post_request(const char *url, const char *post_data, char *output_buffer, int buffer_size) {
    esp_err_t err = ESP_OK;
    http_error_t error_code = HTTP_SUCCESS;

    esp_http_client_set_url(client, url);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    err = esp_http_client_open(client, strlen(post_data));

    if (err != ESP_OK) {
        error_code = HTTP_ERROR_OPEN_CONNECTION;
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            error_code = HTTP_ERROR_WRITE_FAILED;
        } else {
            int content_length = esp_http_client_fetch_headers(client);
            if (content_length < 0) {
                error_code = HTTP_ERROR_FETCH_HEADERS_FAILED;
            } else {
                int data_read = esp_http_client_read_response(client, output_buffer, buffer_size);
                if (data_read >= 0) {
                    error_code = HTTP_SUCCESS;
                } else {
                    error_code = HTTP_ERROR_READ_RESPONSE_FAILED;
                }
            }
        }
    }
    return error_code;
}

esp_err_t api_get_word(char* word, int word_size) {
    if (word == NULL)
    {
        ESP_LOGE(TAG, "Word Pointer is Null");
        return ESP_ERR_INVALID_ARG;
    }
    else if (word_size < WORD_SIZE)
    {
        ESP_LOGE(TAG, "Given size is less than WORD_SIZE");
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGI(TAG, "Sending POST request to URL: %s", GET_WORD_URL);

    const char *post_data = "{\"timezone\":\"UTC + 8\"}";
    char output_buffer[BUFFER_SIZE] = {0};   // Buffer to store response of HTTP request
    http_error_t error_code = make_post_request(GET_WORD_URL, post_data, output_buffer, BUFFER_SIZE);

    switch (error_code) {
        case HTTP_SUCCESS:
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
            ESP_LOG_BUFFER_HEXDUMP(TAG, output_buffer, strlen(output_buffer), ESP_LOG_DEBUG); //Debug line for hexdump and status codes
            // Process the response to extract the word
            // Replace this placeholder with actual parsing logic
            extract_word(output_buffer, strlen(output_buffer), word, word_size);
            printf("Extracted word: %s\n", word);
            break;

        case HTTP_ERROR_OPEN_CONNECTION:
            ESP_LOGE(TAG, "Failed to open HTTP connection");
            word = NULL;
            break;

        case HTTP_ERROR_WRITE_FAILED:
            ESP_LOGE(TAG, "Write failed");
            word = NULL;
            break;

        case HTTP_ERROR_FETCH_HEADERS_FAILED:
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
            word = NULL;
            break;

        case HTTP_ERROR_READ_RESPONSE_FAILED:
            ESP_LOGE(TAG, "Failed to read response");
            word = NULL;
            break;

        default:
            ESP_LOGE(TAG, "An unknown error occurred");
            word = NULL;
            break;
    }

    //esp_http_client_cleanup(client);
    return ESP_OK;
}

    esp_err_t api_check_word(char* guess, int guess_size) {
    char word_guess[5] = {0};
    
    if (guess == NULL)
    {
        ESP_LOGE(TAG, "Guess Pointer is Null");
        return ESP_ERR_INVALID_ARG;
    }
    else if (guess_size < WORD_SIZE)
    {
        ESP_LOGE(TAG, "Given size is less than WORD_SIZE");
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(word_guess, guess, 5);
    ESP_LOGI(TAG, "Sending POST request to URL: %s", CHECK_WORD_URL);
    ESP_LOG_BUFFER_HEXDUMP(TAG, word_guess, 6, ESP_LOG_DEBUG);
    
    char post_data[50] = {0};
    snprintf(post_data, 50, "{\"word\":\"%s\",\"timezone\":\"UTC + 8\"}", word_guess);
    ESP_LOG_BUFFER_HEXDUMP(TAG, post_data, 50, ESP_LOG_DEBUG);
    char output_buffer[BUFFER_SIZE] = {0};   // Buffer to store response of HTTP request
    http_error_t error_code = make_post_request(CHECK_WORD_URL, post_data, output_buffer, BUFFER_SIZE);
    ESP_LOGI(TAG, "Entering api_check_word Switch Statement");

    switch (error_code) {
        case HTTP_SUCCESS:
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
            ESP_LOG_BUFFER_HEXDUMP(TAG, output_buffer, strlen(output_buffer), ESP_LOG_DEBUG);
            // Process the response to extract the result
            // Replace this placeholder with actual parsing logic
            extract_result(output_buffer, strlen(output_buffer), guess, guess_size);

            break;

        case HTTP_ERROR_OPEN_CONNECTION:
            ESP_LOGE(TAG, "Failed to open HTTP connection");
            break;

        case HTTP_ERROR_WRITE_FAILED:
            ESP_LOGE(TAG, "Write failed");
            break;

        case HTTP_ERROR_FETCH_HEADERS_FAILED:
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
            break;

        case HTTP_ERROR_READ_RESPONSE_FAILED:
            ESP_LOGE(TAG, "Failed to read response");
            break;

        default:
            ESP_LOGE(TAG, "An unknown error occurred");
            break;
    }
    ESP_LOGI(TAG, "Post api_check_word Switch Statement");
    //esp_http_client_cleanup(client);
    return ESP_OK;
}

esp_err_t api_client_init(void){

    response.buffer_size = BUFFER_SIZE; // Initial buffer size
    response.buffer = malloc(response.buffer_size);
    if (response.buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
        return ESP_ERR_NO_MEM;
    }
    response.data_length = 0;
    response.buffer[0] = '\0';

    // Configure the HTTP client
    config.url = "https://wordle-game-api1.p.rapidapi.com";
    config.cert_pem = root_cert;
    config.buffer_size = BUFFER_SIZE;

    client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client handle");
        free(response.buffer);
        return ESP_FAIL;
    }
    // Set the request headers
    esp_http_client_set_header(client, "x-rapidapi-key", API_KEY);
    esp_http_client_set_header(client, "x-rapidapi-host", "wordle-game-api1.p.rapidapi.com");
    esp_http_client_set_header(client, "Content-Type", "application/json");

    return ESP_OK;
}

static void extract_word(const char *json_string, size_t json_size, char *word_buffer, size_t buffer_size) {
    cJSON *root = cJSON_ParseWithLength(json_string, json_size);
    if (root == NULL) {
        // Handle JSON parsing error
        word_buffer[0] = '\0';
        return;
    }

    cJSON *word_item = cJSON_GetObjectItem(root, "word");
    if (cJSON_IsString(word_item) && (word_item->valuestring != NULL)) {
        // Copy the "word" value into the provided buffer
        strncpy(word_buffer, word_item->valuestring, buffer_size - 1);
        word_buffer[buffer_size - 1] = '\0';  // Ensure null-termination
    } else {
        // "word" item not found or not a string
        word_buffer[0] = '\0';
    }

    cJSON_Delete(root);
}

static void extract_result(const char *json_string, size_t json_size, char *word_buffer, size_t buffer_size) {
    cJSON *root = cJSON_ParseWithLength(json_string, json_size);
    if (root == NULL) {
        // Handle JSON parsing error
        word_buffer[0] = '\0';
        return;
    }

    cJSON *word_item = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(word_item) && (word_item->valuestring != NULL)) {
        // Copy the "word" value into the provided buffer
        strncpy(word_buffer, word_item->valuestring, buffer_size - 1);
        word_buffer[buffer_size - 1] = '\0';  // Ensure null-termination
    } else {
        // "word" item not found or not a string
        word_buffer[0] = '\0';
    }

    cJSON_Delete(root);
}