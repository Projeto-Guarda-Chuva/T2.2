#pragma once

#include <stddef.h>
#include "esp_err.h"

typedef enum {
    HTTP_GET,
    HTTP_POST,
} httpd_method_t;

typedef struct {
    int   content_len;
    void *user_ctx;
} httpd_req_t;

typedef void *httpd_handle_t;

typedef struct {
    int _unused;
} httpd_config_t;

typedef struct {
    const char    *uri;
    httpd_method_t method;
    esp_err_t    (*handler)(httpd_req_t *r);
    void          *user_ctx;
} httpd_uri_t;

#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){0})

esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config);
esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *uri_handler);
int       httpd_req_recv(httpd_req_t *req, char *buf, size_t buf_len);
esp_err_t httpd_resp_sendstr(httpd_req_t *req, const char *str);
esp_err_t httpd_resp_send_500(httpd_req_t *req);

typedef struct {
    int         start_count;
    int         register_count;
    httpd_uri_t last_uri;
    char        last_uri_path[64];
    int         req_recv_count;
    char        next_body[256];
    int         sendstr_count;
    char        last_response[256];
    int         send_500_count;
    int         process_command_count;
    char        last_command[256];
} http_spy_t;

extern http_spy_t g_http_spy;
void http_spy_reset(void);