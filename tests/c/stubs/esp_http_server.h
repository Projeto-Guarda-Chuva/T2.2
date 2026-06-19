#pragma once

#include <stddef.h>

#include "esp_err.h"

typedef void *httpd_handle_t;
typedef struct httpd_req httpd_req_t;
typedef esp_err_t (*httpd_uri_func_t)(httpd_req_t *req);

typedef struct {
    int server_port;
} httpd_config_t;

typedef struct {
    const char *uri;
    int method;
    httpd_uri_func_t handler;
    void *user_ctx;
} httpd_uri_t;

struct httpd_req {
    size_t content_len;
    void *user_ctx;
};

#define HTTP_POST 1
#define HTTP_GET 2
#define HTTPD_500_INTERNAL_SERVER_ERROR 500
#define HTTPD_RESP_USE_STRLEN ((int)-1)
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){.server_port = 80})

esp_err_t httpd_start(httpd_handle_t *handle, httpd_config_t *config);
esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *uri_handler);
esp_err_t httpd_resp_send(httpd_req_t *req, const char *buf, int buf_len);
esp_err_t httpd_resp_sendstr(httpd_req_t *req, const char *str);
esp_err_t httpd_resp_send_err(httpd_req_t *req, int error, const char *message);
esp_err_t httpd_resp_send_500(httpd_req_t *req);
int httpd_req_recv(httpd_req_t *req, char *buf, size_t buf_len);
