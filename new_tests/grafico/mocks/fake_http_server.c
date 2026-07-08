#include "esp_http_server.h"
#include "protocol.h"

#include <string.h>
#include <stdio.h>

http_spy_t g_http_spy;

void http_spy_reset(void) {
    memset(&g_http_spy, 0, sizeof(g_http_spy));
}

esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config) {
    (void)config;
    g_http_spy.start_count++;
    if (handle) {
        *handle = (httpd_handle_t)1;
    }
    
    return ESP_OK;
}

esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *uri_handler) {
    (void)handle;
    g_http_spy.register_count++;
    if (uri_handler) {
        g_http_spy.last_uri = *uri_handler; 
        
        if (uri_handler->uri) {
            snprintf(g_http_spy.last_uri_path, sizeof(g_http_spy.last_uri_path), "%s", uri_handler->uri);
            g_http_spy.last_uri.uri = g_http_spy.last_uri_path;
        }
    }
    
    return ESP_OK;
}

int httpd_req_recv(httpd_req_t *req, char *buf, size_t buf_len) {
    (void)req;
    g_http_spy.req_recv_count++;
    size_t n = strlen(g_http_spy.next_body);
    
    if (n > buf_len) {
        n = buf_len;
    }
    memcpy(buf, g_http_spy.next_body, n);
    
    return (int)n;
}

esp_err_t httpd_resp_sendstr(httpd_req_t *req, const char *str) {
    (void)req;
    g_http_spy.sendstr_count++;
    snprintf(g_http_spy.last_response, sizeof(g_http_spy.last_response), "%s", str ? str : "");
    
    return ESP_OK;
}

esp_err_t httpd_resp_send_500(httpd_req_t *req) {
    (void)req;
    g_http_spy.send_500_count++;

    return ESP_OK;
}

void process_command_json(const char *json_str) {
    g_http_spy.process_command_count++;
    snprintf(g_http_spy.last_command, sizeof(g_http_spy.last_command), "%s", json_str ? json_str : "");
}
