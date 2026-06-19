#pragma once

typedef struct cJSON {
    char *raw_json;
    int valueint;
    int is_number;
} cJSON;

cJSON *cJSON_CreateObject(void);
cJSON *cJSON_AddNumberToObject(cJSON *object, const char *key, double value);
cJSON *cJSON_AddStringToObject(cJSON *object, const char *key, const char *value);
char *cJSON_PrintUnformatted(const cJSON *item);
cJSON *cJSON_Parse(const char *json_str);
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *key);
int cJSON_IsNumber(const cJSON *item);
void cJSON_Delete(cJSON *item);
