#include "cJSON.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static cJSON temp_item;

cJSON *cJSON_CreateObject(void) {
    cJSON *root = calloc(1, sizeof(*root));
    return root;
}

cJSON *cJSON_AddNumberToObject(cJSON *object, const char *key, double value) {
    (void)object;
    (void)key;
    (void)value;
    return NULL;
}

cJSON *cJSON_AddStringToObject(cJSON *object, const char *key, const char *value) {
    (void)object;
    (void)key;
    (void)value;
    return NULL;
}

char *cJSON_PrintUnformatted(const cJSON *item) {
    const char *payload = "{\"status\":\"running\"}";
    size_t len;
    char *buffer;
    (void)item;
    len = strlen(payload);
    buffer = malloc(len + 1);
    if (buffer == NULL) {
        return NULL;
    }
    memcpy(buffer, payload, len + 1);
    return buffer;
}

static int parse_int_after_colon(const char *position, int *out_value) {
    char *endptr;

    while (*position && *position != ':') {
        position++;
    }

    if (*position != ':') {
        return 0;
    }

    position++;
    while (*position && isspace((unsigned char)*position)) {
        position++;
    }

    *out_value = (int)strtol(position, &endptr, 10);
    return endptr != position;
}

cJSON *cJSON_Parse(const char *json_str) {
    cJSON *root;
    size_t len;

    if (json_str == NULL || json_str[0] != '{') {
        return NULL;
    }

    root = calloc(1, sizeof(*root));
    if (root == NULL) {
        return NULL;
    }

    len = strlen(json_str);
    root->raw_json = malloc(len + 1);
    if (root->raw_json == NULL) {
        free(root);
        return NULL;
    }
    memcpy(root->raw_json, json_str, len + 1);

    return root;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *key) {
    char pattern[32];
    const char *position;
    int value;

    if (object == NULL || object->raw_json == NULL || key == NULL) {
        return NULL;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    position = strstr(object->raw_json, pattern);
    if (position == NULL) {
        return NULL;
    }

    if (!parse_int_after_colon(position, &value)) {
        return NULL;
    }

    temp_item.valueint = value;
    temp_item.is_number = 1;
    return &temp_item;
}

int cJSON_IsNumber(const cJSON *item) {
    return item != NULL && item->is_number;
}

void cJSON_Delete(cJSON *item) {
    if (item == NULL) {
        return;
    }

    free(item->raw_json);
    free(item);
}
