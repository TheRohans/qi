#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjson/cJSON.h"

void log_message(const char *message) {
    fprintf(stderr, "[server] %s\n", message);
}

void send_response(const char *id, cJSON *result) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    cJSON_AddStringToObject(response, "id", id);
    cJSON_AddItemToObject(response, "result", result);

    char *response_str = cJSON_Print(response);
    log_message("Sending response");
    log_message(response_str);
    fprintf(stdout, "Content-Length: %lu\r\n\r\n", strlen(response_str));
    fprintf(stdout, "%s", response_str);
    fflush(stdout);

    cJSON_Delete(response);
    free(response_str);
}

void send_error_response(const char *id, int code, const char *message) {
    cJSON *error = cJSON_CreateObject();
    cJSON_AddNumberToObject(error, "code", code);
    cJSON_AddStringToObject(error, "message", message);

    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    cJSON_AddStringToObject(response, "id", id);
    cJSON_AddItemToObject(response, "error", error);

    char *response_str = cJSON_Print(response);
    log_message("Sending error response");
    log_message(response_str); // Log the sent error response
    printf("Content-Length: %lu\r\n\r\n", strlen(response_str));
    printf("%s", response_str);
    fflush(stdout);

    cJSON_Delete(response);
    free(response_str);
}

void handle_initialize(cJSON *params) {
    log_message("Handling initialize request");
    // Respond to initialize request
    cJSON *capabilities = cJSON_CreateObject();
    cJSON *response_result = cJSON_CreateObject();
    cJSON_AddItemToObject(response_result, "capabilities", capabilities);
    send_response("1", response_result);
}

void handle_hover(cJSON *params) {
    log_message("Handling hover request");
    // Extract textDocumentUri, positionLine, and positionCharacter from params
    cJSON *uri = cJSON_GetObjectItemCaseSensitive(params, "textDocumentUri");
    cJSON *line = cJSON_GetObjectItemCaseSensitive(params, "positionLine");
    cJSON *character = cJSON_GetObjectItemCaseSensitive(params, "positionCharacter");

    if (uri && line && character) {
        // Create a mock response with the contents of the line
        cJSON *response_result = cJSON_CreateObject();
        char contents[256];
        snprintf(contents, sizeof(contents), "Line %d, Character %d: Content at %s", line->valueint, character->valueint, uri->valuestring);
        cJSON_AddStringToObject(response_result, "contents", contents);
        send_response("1", response_result);
    } else {
        // Send an error response if parameters are missing
        send_error_response("1", -32602, "Invalid params");
    }
}

int main() {
    log_message("Starting up");
    log_message("Listening on stdin/stdout");
    log_message("Ready for connection");

    char header[256];
    while (fgets(header, sizeof(header), stdin) != NULL) {
        log_message("Message received");
        log_message(header);

        size_t content_length = 0;
        if (sscanf(header, "Content-Length: %lu\r\n\r\n", &content_length) == 1) {
            char *request_buffer = (char *)malloc(content_length + 1);
            if (request_buffer == NULL) {
                perror("malloc");
                continue;
            }

            // size_t total_read = 0;
            // while (total_read < (size_t)content_length) {
            //     size_t n = fread(request_buffer + total_read, 1, content_length - total_read, stdin);
            //     if (n == 0) break; // EOF or error
            //     total_read += n;
            // }
            // request_buffer[total_read] = '\0';

            fread(request_buffer, sizeof(char), content_length, stdin);
            request_buffer[content_length] = '\0';

            char testing[256];
            sprintf(testing, "-->%s<--", request_buffer);
            log_message(testing);

            log_message("Request body received");
            log_message(request_buffer);

            cJSON *request = cJSON_Parse(request_buffer);
            log_message(cJSON_Print(request));

            if (request) {
                cJSON *method = cJSON_GetObjectItemCaseSensitive(request, "method");
                cJSON *params = cJSON_GetObjectItemCaseSensitive(request, "params");
                if (method && params) {
                    if (strcmp(method->valuestring, "initialize") == 0) {
                        handle_initialize(params);
                    } else if (strcmp(method->valuestring, "textDocument/hover") == 0) {
                        handle_hover(params);
                    }
                }
                cJSON_Delete(request);
            }

            free(request_buffer);
        } else {
            log_message("Failed to parse Content-Length header.");
        }
    }


    log_message("shutting down...");
    return 0;
}
