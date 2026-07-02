#include "unity.h"
#include "protocol.h"
#include "fake_display.h"
#include "test_log_capture.h"
#include "test_results.h"

#include <stdio.h>

void setUp(void) {
    display_spy_reset();
    test_log_reset();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);

    char line[128];
    if (g_display_spy.call_count > 0) {
        snprintf(line, sizeof(line), "display_set_bg_color(%u, %u, %u) chamado %dx",
                 g_display_spy.last_r, g_display_spy.last_g, g_display_spy.last_b,
                 g_display_spy.call_count);
    } else {
        snprintf(line, sizeof(line), "display nao foi chamado");
    }
    results_add_output(line);

    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

void test_id11_vermelho(void) {
    results_set_input("{\"id\":11}");
    process_command_json("{\"id\":11}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 11 deveria acionar o display exatamente uma vez.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_r, "id 11 deveria pintar vermelho (R=255).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g, "id 11 deveria pintar vermelho (G=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 11 deveria pintar vermelho (B=0).");
}

void test_id15_branco(void) {
    results_set_input("{\"id\":15}");
    process_command_json("{\"id\":15}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 15 deveria acionar o display exatamente uma vez.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_r, "id 15 deveria pintar branco (R=255).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_g, "id 15 deveria pintar branco (G=255).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_b, "id 15 deveria pintar branco (B=255).");
}

void test_id12_verde(void) {
    results_set_input("{\"id\":12}");
    process_command_json("{\"id\":12}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 12 deveria acionar o display exatamente uma vez.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_r, "id 12 deveria pintar verde (R=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_g, "id 12 deveria pintar verde (G=255).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 12 deveria pintar verde (B=0).");
}

void test_id13_azul(void) {
    results_set_input("{\"id\":13}");
    process_command_json("{\"id\":13}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 13 deveria acionar o display exatamente uma vez.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_r, "id 13 deveria pintar azul (R=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g, "id 13 deveria pintar azul (G=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_b, "id 13 deveria pintar azul (B=255).");
}

void test_id14_preto(void) {
    results_set_input("{\"id\":14}");
    process_command_json("{\"id\":14}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 14 deveria acionar o display exatamente uma vez.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_r, "id 14 deveria pintar preto (R=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g, "id 14 deveria pintar preto (G=0).");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 14 deveria pintar preto (B=0).");
}

void test_id10_cor_customizada(void) {
    results_set_input("{\"id\":10,\"r\":12,\"g\":34,\"b\":56}");
    process_command_json("{\"id\":10,\"r\":12,\"g\":34,\"b\":56}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 10 deveria acionar o display com a cor informada.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(12, g_display_spy.last_r, "id 10 deveria usar R=12.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(34, g_display_spy.last_g, "id 10 deveria usar G=34.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(56, g_display_spy.last_b, "id 10 deveria usar B=56.");
}

void test_id10_sem_campos(void) {
    results_set_input("{\"id\":10}");
    process_command_json("{\"id\":10}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 10 sem r/g/b deveria acionar o display assumindo tudo 0.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_r, "id 10 sem r deveria assumir R=0.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g, "id 10 sem g deveria assumir G=0.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 10 sem b deveria assumir B=0.");
}

void test_id10_apenas_g(void) {
    results_set_input("{\"id\":10,\"g\":50}");
    process_command_json("{\"id\":10,\"g\":50}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 10 com apenas g deveria acionar o display.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_r, "id 10 sem r deveria assumir R=0.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(50, g_display_spy.last_g, "id 10 deveria usar G=50.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 10 sem b deveria assumir B=0.");
}

void test_json_invalido(void) {
    results_set_input("isso nao e json");
    process_command_json("isso nao e json");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_display_spy.call_count,
        "JSON invalido nao deveria acionar o display.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "JSON invalido deveria registrar erro no log.");
}

void test_sem_id(void) {
    results_set_input("{\"cor\":\"vermelho\"}");
    process_command_json("{\"cor\":\"vermelho\"}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_display_spy.call_count,
        "Comando sem 'id' nao deveria acionar o display.");
}

void test_id_tipo_errado(void) {
    results_set_input("{\"id\":\"abc\"}");
    process_command_json("{\"id\":\"abc\"}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_display_spy.call_count,
        "'id' nao-numerico nao deveria acionar o display.");
}

void test_id_desconhecido(void) {
    results_set_input("{\"id\":99}");
    process_command_json("{\"id\":99}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_display_spy.call_count,
        "id desconhecido nao deveria acionar o display.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('W') > 0,
        "id desconhecido deveria registrar warning no log.");
}

void test_id10_campos_faltando(void) {
    results_set_input("{\"id\":10,\"r\":200}");
    process_command_json("{\"id\":10,\"r\":200}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_display_spy.call_count,
        "id 10 deveria acionar o display mesmo com campos faltando.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(200, g_display_spy.last_r, "id 10 deveria usar R=200.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g, "id 10 deveria assumir G=0 quando ausente.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_b, "id 10 deveria assumir B=0 quando ausente.");
}

void test_id10_rgb_fora_da_faixa(void) {
    results_set_input("{\"id\":10,\"r\":300,\"g\":-5,\"b\":0}");
    process_command_json("{\"id\":10,\"r\":300,\"g\":-5,\"b\":0}");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(255, g_display_spy.last_r,
        "R=300 deveria ser cortado para 255: falta validacao de faixa (0-255) em process_command_json.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_display_spy.last_g,
        "G=-5 deveria ser cortado para 0: falta validacao de faixa (0-255) em process_command_json.");
}

void test_comando_de_outro_grupo(void) {
    results_set_input("{\"id\":21}");
    process_command_json("{\"id\":21}");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_display_spy.call_count,
        "Comando de outro atuador (id 21, motor) nao deveria acionar o display.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "grafico.json";

    results_begin("grafico");

    UNITY_BEGIN();
    RUN_TEST(test_id11_vermelho);
    RUN_TEST(test_id12_verde);
    RUN_TEST(test_id13_azul);
    RUN_TEST(test_id14_preto);
    RUN_TEST(test_id15_branco);
    RUN_TEST(test_id10_cor_customizada);
    RUN_TEST(test_id10_sem_campos);
    RUN_TEST(test_id10_apenas_g);
    RUN_TEST(test_json_invalido);
    RUN_TEST(test_sem_id);
    RUN_TEST(test_id_tipo_errado);
    RUN_TEST(test_id_desconhecido);
    RUN_TEST(test_id10_campos_faltando);
    RUN_TEST(test_id10_rgb_fora_da_faixa);
    RUN_TEST(test_comando_de_outro_grupo);
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
