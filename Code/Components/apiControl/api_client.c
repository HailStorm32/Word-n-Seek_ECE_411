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



// Event handler for HTTP events
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    response_data_t *eventResponse = (response_data_t *)evt->user_data;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            break;
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len > 0) {
                // Ensure there's enough space in the buffer
                if (eventResponse->data_length + evt->data_len >= eventResponse->buffer_size) {
                    // Expand the buffer
                    int new_size = eventResponse->buffer_size + evt->data_len + 1; // +1 for null terminator
                    char *new_buffer = realloc(eventResponse->buffer, new_size);
                    if (new_buffer == NULL) {
                        ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
                        return ESP_FAIL;
                    }
                    eventResponse->buffer = new_buffer;
                    eventResponse->buffer_size = new_size;
                }
                // Copy the data into the buffer
                memcpy(eventResponse->buffer + eventResponse->data_length, evt->data, evt->data_len);
                eventResponse->data_length += evt->data_len;
                eventResponse->buffer[eventResponse->data_length] = '\0'; // Null-terminate the buffer
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}



char* api_get_word() {
    char* word = malloc(WORD_SIZE);
    if (word == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for word");
        return NULL;
    }

    ESP_LOGI(TAG, "Sending GET request to URL: %s", GET_WORD_URL);

    const char *post_data = "{\"timezone\":\"UTC + 8\"}";
    char output_buffer[BUFFER_SIZE] = {0};   // Buffer to store response of http request
    int content_length = 0;
    esp_err_t err = ESP_OK;
    http_error_t error_code = HTTP_SUCCESS;

    esp_http_client_set_url(client, GET_WORD_URL);
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
            content_length = esp_http_client_fetch_headers(client);
            if (content_length < 0) {
                error_code = HTTP_ERROR_FETCH_HEADERS_FAILED;
            } else {
                int data_read = esp_http_client_read_response(client, output_buffer, BUFFER_SIZE);
                if (data_read >= 0) {
                    error_code = HTTP_SUCCESS;
                } else {
                    error_code = HTTP_ERROR_READ_RESPONSE_FAILED;
                }
            }
        }
    }

    switch (error_code) {
        case HTTP_SUCCESS:
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
            ESP_LOG_BUFFER_HEXDUMP(TAG, output_buffer, strlen(output_buffer), ESP_LOG_INFO);
            // Process the response to extract the word
            // Replace this placeholder with actual parsing logic
            memcpy(word, "word", WORD_SIZE);
            break;

        case HTTP_ERROR_OPEN_CONNECTION:
            ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
            free(word);
            word = NULL;
            break;

        case HTTP_ERROR_WRITE_FAILED:
            ESP_LOGE(TAG, "Write failed");
            free(word);
            word = NULL;
            break;

        case HTTP_ERROR_FETCH_HEADERS_FAILED:
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
            free(word);
            word = NULL;
            break;

        case HTTP_ERROR_READ_RESPONSE_FAILED:
            ESP_LOGE(TAG, "Failed to read response");
            free(word);
            word = NULL;
            break;

        default:
            ESP_LOGE(TAG, "An unknown error occurred");
            free(word);
            word = NULL;
            break;
    }

    esp_http_client_cleanup(client);
    return word;
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
    //config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    //config.skip_cert_common_name_check = true;
    config.buffer_size = BUFFER_SIZE;
    // .method = HTTP_METHOD_POST,
    //config.event_handler = _http_event_handler;
    //config.user_data = &response; // Pass response buffer to event handler
    

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


