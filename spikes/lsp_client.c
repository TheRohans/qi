#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cjson/cJSON.h"

void send_json_request(FILE *fp, const char *method, cJSON *params) {
    cJSON *request = cJSON_CreateObject();
    cJSON_AddStringToObject(request, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(request, "id", 1);
    cJSON_AddStringToObject(request, "method", method);
    cJSON_AddItemToObject(request, "params", params);

    char *request_str = cJSON_Print(request);
    fprintf(fp, "%s\n", request_str);
    fflush(fp);

    cJSON_Delete(request);
    free(request_str);
}

void receive_json_response(FILE *fp) {
    char buffer[1024] = {0};
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        cJSON *response = cJSON_Parse(buffer);
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
    }
}

int main() {
    FILE *fp = popen("/usr/bin/clangd", "r+");
    if (fp == NULL) {
        perror("popen");
        return EXIT_FAILURE;
    }

    cJSON *params = cJSON_CreateObject();
    cJSON_AddStringToObject(params, "textDocumentUri", "file:///Users/robrohan/Projects/TheRohans/qi/spikes/qstack.c");
    cJSON_AddNumberToObject(params, "positionLine", 7);
    cJSON_AddNumberToObject(params, "positionCharacter", 10);

    send_json_request(fp, "textDocument/hover", params);
    receive_json_response(fp);

    cJSON_Delete(params);
    pclose(fp);
    return 0;
}
