#include "http_server.h"
#include "protocol.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"

static esp_err_t comando_post_handler(httpd_req_t *req) {
    char buf[128];
    int remaining = req->content_len;

    if ((size_t)remaining >= sizeof(buf)) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    int ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0'; 

    cJSON *checker = cJSON_Parse(buf);
    if (!checker) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON_Delete(checker);

    process_command_json(buf); 

    httpd_resp_sendstr(req, "{\"status\": \"ok\"}");
    return ESP_OK;
}


void start_web_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_post = {
            .uri      = "/cmd",
            .method   = HTTP_POST,
            .handler  = comando_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_post);
        ESP_LOGI("HTTP_SERVER", "Servidor rodando na porta 80");
    }
}
