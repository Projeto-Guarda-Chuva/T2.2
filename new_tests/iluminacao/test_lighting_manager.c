#include "unity.h"
#include "lighting_manager.h"
#include "led_strip.h"
#include "test_log_capture.h"
#include "test_results.h"

#include <stdio.h>

void setUp(void) {
    led_spy_reset();
    test_log_reset();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);

    char line[192];
    snprintf(line, sizeof(line),
             "set_pixel=%dx ultima=(%u,%u,%u) refresh=%dx clear=%dx new_device=%dx",
             g_led_spy.set_pixel_count, g_led_spy.last_r, g_led_spy.last_g, g_led_spy.last_b,
             g_led_spy.refresh_count, g_led_spy.clear_count, g_led_spy.new_device_count);
    results_add_output(line);

    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

void test_set_color_antes_do_init_erro(void) {
    results_set_input("lighting_set_color(255,0,0) sem lighting_init()");
    lighting_set_color(255, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_led_spy.set_pixel_count,
        "set_color sem init nao deveria escrever em nenhum LED.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "set_color sem init deveria registrar erro no log.");
}

void test_off_antes_do_init_erro(void) {
    results_set_input("lighting_off() sem lighting_init()");
    lighting_off();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_led_spy.clear_count,
        "off sem init nao deveria limpar a fita.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "off sem init deveria registrar erro no log.");
}

void test_set_color_antes_do_init_nao_deveria_aceitar(void) {
    results_set_input("lighting_set_color(100,0,0) sem lighting_init()");
    lighting_set_color(100, 0, 0);
    TEST_ASSERT_FALSE_MESSAGE(test_log_contains("Nova cor recebida"),
        "set_color deveria validar a inicializacao antes de processar: sem a fita, nao deveria aceitar/anunciar a cor.");
}

void test_init_cria_a_fita(void) {
    results_set_input("lighting_init()");
    lighting_init();
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_led_spy.new_device_count,
        "init deveria criar o driver da fita exatamente uma vez.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('I') > 0,
        "init deveria registrar mensagens de inicializacao.");
}

void test_init_repetido_nao_recria(void) {
    results_set_input("lighting_init() com a fita ja inicializada");
    lighting_init();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_led_spy.new_device_count,
        "init repetido nao deveria recriar o driver da fita.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('W') > 0,
        "init repetido deveria registrar warning de 'ja inicializado'.");
}

void test_set_color_pinta_24_leds(void) {
    results_set_input("lighting_set_color(10,20,30)");
    lighting_set_color(10, 20, 30);
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, g_led_spy.set_pixel_count,
        "set_color deveria escrever nos 24 LEDs.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(10, g_led_spy.last_r, "R aplicado deveria ser 10.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(20, g_led_spy.last_g, "G aplicado deveria ser 20.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(30, g_led_spy.last_b, "B aplicado deveria ser 30.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_led_spy.refresh_count,
        "set_color deveria dar refresh na fita uma vez.");
}

void test_off_apaga_a_fita(void) {
    results_set_input("lighting_off()");
    lighting_off();
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_led_spy.clear_count,
        "off deveria limpar a fita uma vez.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_led_spy.refresh_count,
        "off deveria dar refresh na fita uma vez.");
}

void test_set_intensity_reaplica_escalado(void) {
    results_set_input("lighting_set_color(200,100,50); lighting_set_intensity(128)");
    lighting_set_color(200, 100, 50);
    led_spy_reset();
    lighting_set_intensity(128);
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, g_led_spy.set_pixel_count,
        "mudar a intensidade deveria reaplicar a cor nos 24 LEDs.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(100, g_led_spy.last_r, "R escalado por 128 deveria ser 100.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(50, g_led_spy.last_g, "G escalado por 128 deveria ser 50.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(25, g_led_spy.last_b, "B escalado por 128 deveria ser 25.");
}

void test_intensidade_zero_apaga_a_cor(void) {
    results_set_input("lighting_set_color(255,255,255); lighting_set_intensity(0)");
    lighting_set_color(255, 255, 255);
    led_spy_reset();
    lighting_set_intensity(0);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_led_spy.last_r, "intensidade 0 deveria zerar R.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_led_spy.last_g, "intensidade 0 deveria zerar G.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, g_led_spy.last_b, "intensidade 0 deveria zerar B.");
}

void test_intensidade_maxima_mantem_a_cor(void) {
    results_set_input("lighting_set_intensity(255); lighting_set_color(80,120,200)");
    lighting_set_intensity(255);
    led_spy_reset();
    lighting_set_color(80, 120, 200);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(80, g_led_spy.last_r, "intensidade 255 deveria manter R=80.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(120, g_led_spy.last_g, "intensidade 255 deveria manter G=120.");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(200, g_led_spy.last_b, "intensidade 255 deveria manter B=200.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "iluminacao.json";

    results_begin("iluminacao");

    UNITY_BEGIN();
    RUN_TEST(test_set_color_antes_do_init_erro);
    RUN_TEST(test_off_antes_do_init_erro);
    RUN_TEST(test_set_color_antes_do_init_nao_deveria_aceitar);
    RUN_TEST(test_init_cria_a_fita);
    RUN_TEST(test_init_repetido_nao_recria);
    RUN_TEST(test_set_color_pinta_24_leds);
    RUN_TEST(test_off_apaga_a_fita);
    RUN_TEST(test_set_intensity_reaplica_escalado);
    RUN_TEST(test_intensidade_zero_apaga_a_cor);
    RUN_TEST(test_intensidade_maxima_mantem_a_cor);
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
