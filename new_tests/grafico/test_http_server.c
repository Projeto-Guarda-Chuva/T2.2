#include "unity.h"
#include "http_server.h"
#include "esp_http_server.h"
#include "test_log_capture.h"
#include "test_results.h"

#include <stdio.h>
#include <string.h>

void setUp(void) {
    http_spy_reset();
    test_log_reset();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);

    char line[400];
    snprintf(line, sizeof(line),
             "process_command=%dx ultimo='%s' resposta='%s' send_500=%dx",
             g_http_spy.process_command_count,
             g_http_spy.last_command,
             g_http_spy.sendstr_count ? g_http_spy.last_response : "(nenhuma)",
             g_http_spy.send_500_count);
    results_add_output(line);

    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

void test_registra_endpoint_cmd(void) {
    results_set_input("start_web_server()");
    start_web_server();
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_http_spy.start_count,
        "deveria iniciar o servidor HTTP uma vez.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_http_spy.register_count,
        "deveria registrar exatamente um endpoint.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("/cmd", g_http_spy.last_uri.uri,
        "o endpoint registrado deveria ser /cmd.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(HTTP_POST, g_http_spy.last_uri.method,
        "o endpoint /cmd deveria ser do tipo POST.");
    TEST_ASSERT_NOT_NULL_MESSAGE(g_http_spy.last_uri.handler,
        "o endpoint /cmd deveria ter um handler registrado.");
}

void test_payload_valido_encaminhado(void) {
    results_set_input("POST /cmd corpo='{\"id\":11}'");
    start_web_server();
    snprintf(g_http_spy.next_body, sizeof(g_http_spy.next_body), "%s", "{\"id\":11}");
    httpd_req_t req = { .content_len = 9, .user_ctx = NULL };
    esp_err_t ret = g_http_spy.last_uri.handler(&req);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ESP_OK, ret,
        "payload valido deveria retornar ESP_OK.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_http_spy.process_command_count,
        "o corpo deveria ser encaminhado ao process_command_json.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"id\":11}", g_http_spy.last_command,
        "o corpo encaminhado deveria ser preservado.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"status\": \"ok\"}", g_http_spy.last_response,
        "deveria responder ok para um payload valido.");
}

void test_corpo_no_limite_128_rejeitado(void) {
    results_set_input("POST /cmd content_len=128");
    start_web_server();
    httpd_req_t req = { .content_len = 128, .user_ctx = NULL };
    esp_err_t ret = g_http_spy.last_uri.handler(&req);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ESP_FAIL, ret,
        "corpo de 128 bytes (>= buffer) deveria ser rejeitado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_http_spy.send_500_count,
        "corpo grande demais deveria responder 500.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_http_spy.process_command_count,
        "corpo rejeitado nao deveria ser encaminhado.");
}

void test_corpo_127_aceito(void) {
    results_set_input("POST /cmd content_len=127 corpo='{}'");
    start_web_server();
    snprintf(g_http_spy.next_body, sizeof(g_http_spy.next_body), "%s", "{}");
    httpd_req_t req = { .content_len = 127, .user_ctx = NULL };
    esp_err_t ret = g_http_spy.last_uri.handler(&req);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ESP_OK, ret,
        "corpo de 127 bytes (< buffer) deveria ser aceito.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_http_spy.process_command_count,
        "corpo aceito deveria ser encaminhado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_http_spy.send_500_count,
        "corpo aceito nao deveria responder 500.");
}

void test_corpo_vazio_falha(void) {
    results_set_input("POST /cmd corpo vazio (content_len=0)");
    start_web_server();
    httpd_req_t req = { .content_len = 0, .user_ctx = NULL };
    esp_err_t ret = g_http_spy.last_uri.handler(&req);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ESP_FAIL, ret,
        "corpo vazio deveria retornar ESP_FAIL.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_http_spy.process_command_count,
        "corpo vazio nao deveria ser encaminhado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_http_spy.sendstr_count,
        "corpo vazio nao envia nenhuma resposta ao cliente.");
}

void test_json_invalido_nao_deveria_responder_ok(void) {
    results_set_input("POST /cmd corpo='isso nao e json'");
    start_web_server();
    snprintf(g_http_spy.next_body, sizeof(g_http_spy.next_body), "%s", "isso nao e json");
    httpd_req_t req = { .content_len = 15, .user_ctx = NULL };
    g_http_spy.last_uri.handler(&req);
    TEST_ASSERT_FALSE_MESSAGE(strcmp(g_http_spy.last_response, "{\"status\": \"ok\"}") == 0,
        "o conector nao deveria responder 'ok' para um comando invalido: o handler responde ok independentemente do conteudo.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "grafico_http.json";

    results_begin("grafico_http");

    UNITY_BEGIN();
    RUN_TEST(test_registra_endpoint_cmd);
    RUN_TEST(test_payload_valido_encaminhado);
    RUN_TEST(test_corpo_no_limite_128_rejeitado);
    RUN_TEST(test_corpo_127_aceito);
    RUN_TEST(test_corpo_vazio_falha);
    RUN_TEST(test_json_invalido_nao_deveria_responder_ok);
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
