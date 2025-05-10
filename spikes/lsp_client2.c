#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "cjson/cJSON.h"

FILE *fp;

void log_message(const char *message) {
    fprintf(stderr, "[client] %s\n", message);
}

void *read_thread(void *arg) {
    log_message("Read thread started");
    char header[256];
    while (fgets(header, sizeof(header), fp) != NULL) {
        log_message("Header received");
        log_message(header);

        // Extract Content-Length
        size_t content_length = 0;
        if (sscanf(header, "Content-Length: %lu\r\n\r\n", &content_length) == 1) {
            char *response_buffer = (char *)malloc(content_length + 1);
            if (response_buffer == NULL) {
                perror("malloc");
                continue;
            }

            // Read the response body
            fread(response_buffer, sizeof(char), content_length, fp);
            response_buffer[content_length] = '\0';

            log_message("Response body received");
            log_message(response_buffer); // Log the received response body

            cJSON *response = cJSON_Parse(response_buffer);
            if (response) {
                cJSON *result = cJSON_GetObjectItemCaseSensitive(response, "result");
                if (result) {
                    char *result_str = cJSON_Print(result);
                    printf("Response: %s\n", result_str);
                    free(result_str);
                } else {
                    printf("No result in response.\n");
                }
                cJSON_Delete(response);
            } else {
                printf("Failed to parse JSON response.\n");
            }

            free(response_buffer);
        } else {
            log_message("Failed to parse Content-Length header.");
        }
    }
    log_message("Read thread finished");
    return NULL;
}

void send_json_request(const char *method, cJSON *params) {
    cJSON *request = cJSON_CreateObject();
    cJSON_AddStringToObject(request, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(request, "id", 1);
    cJSON_AddStringToObject(request, "method", method);
    cJSON_AddItemToObject(request, "params", params);

    char *request_str =  cJSON_PrintUnformatted(request);
    log_message("Sending request");
    // log_message(request_str);

    // Check if fp is valid
    if (fp == NULL) {
        log_message("File pointer is null. Cannot send request.");
        cJSON_Delete(request);
        free(request_str);
        return;
    }

    // send the request
    fprintf(stderr, "Content-Length: %lu\r\n\r\n%s", strlen(request_str), request_str);
    fprintf(fp, "Content-Length: %lu\r\n\r\n%s", strlen(request_str), request_str);
    // fprintf(fp, "%s\n", request_str);
    fflush(fp);

    // Check for errors
    if (ferror(fp)) {
        log_message("Error sending request.");
        clearerr(fp);
    } else {
        log_message("Request sent successfully.");
    }

    cJSON_Delete(request);
    free(request_str);
}

void send_initialize_request() {
    cJSON *params = cJSON_CreateObject();
    cJSON *capabilities = cJSON_Parse("{}");
    cJSON_AddNumberToObject(params, "processId", getpid()); // Use the actual process ID
    cJSON_AddStringToObject(params, "rootUri", "file:///Users/robrohan/Projects/TheRohans/qi/spikes");
    // Add actual capabilities as needed
    // cJSON_AddStringToObject(params, "capabilities", capabilities);
    cJSON_AddItemReferenceToObject(params, "capabilities", capabilities);
    send_json_request("initialize", params);
    cJSON_Delete(params);
}

int main() {
    fp = popen("./lsp_server", "r+"); // Connect to the custom LSP server
    setbuf(fp, NULL);
    if (fp == NULL) {
        perror("popen");
        log_message("open fp error");
        return EXIT_FAILURE;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, read_thread, NULL) != 0) {
        perror("pthread_create");
        log_message("phread_create error");
        pclose(fp);
        return EXIT_FAILURE;
    }

    log_message("Client started");
    // Give the server some time to initialize
    sleep(1);

    // while (1) {
    send_initialize_request();
    log_message("waiting");
    // sleep(100);
    // }

    // cJSON *params = cJSON_CreateObject();
    // cJSON_AddStringToObject(params, "textDocumentUri", "file:///Users/robrohan/Projects/TheRohans/qi/spikes/qstack.c");
    // cJSON_AddNumberToObject(params, "positionLine", 7);
    // cJSON_AddNumberToObject(params, "positionCharacter", 10);
    // send_json_request("textDocument/hover", params);
    // cJSON_Delete(params);

    // Wait for the read thread to finish
    pthread_join(tid, NULL);

    pclose(fp);
    return 0;
}
