#include "unity.h"
#include "motor_fixo.h"
#include "mocks/fake_esp8266.h"
#include "mocks/test_log_capture.h"
#include "../support/test_results.h"

void setUp(void) {
    gpio_spy_reset();
    test_log_reset();
    gate_init();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);

    char line[256];
    snprintf(line, sizeof(line),
             "state=%d pulse=%d BOT_A=%d BOT_B=%d FA=%d FF=%d",
             gate_get_state(), gate_get_pulse_state(),
             gate_get_pin_state(BOT_A), gate_get_pin_state(BOT_B),
             gate_get_pin_state(FA), gate_get_pin_state(FF));
    results_add_output(line);

    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

/* ────────────────────────────────────────────────────────────
 * Testes de Inicialização
 * ──────────────────────────────────────────────────────────── */

void test_gate_init_estado_parado(void) {
    results_set_input("gate_init()");
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "gate_init() deveria deixar o portao no estado PARADO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PULSE_IDLE, gate_get_pulse_state(),
        "gate_init() deveria deixar pulse_state em IDLE.");
}

void test_gate_init_todos_pinos_zerados(void) {
    results_set_input("gate_init() deve zerar todos os pinos");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A), "BOT_A deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B), "BOT_B deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FA), "FA deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FF), "FF deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A_LED), "BOT_A_LED deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B_LED), "BOT_B_LED deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FA_LED), "FA_LED deve iniciar em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FF_LED), "FF_LED deve iniciar em 0.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Abrir Portão
 * ──────────────────────────────────────────────────────────── */

void test_gate_open_ativa_bot_a(void) {
    results_set_input("gate_open()");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(),
        "gate_open() deveria colocar estado em OPENING.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(BOT_A),
        "gate_open() deveria ativar BOT_A.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(BOT_A_LED),
        "gate_open() deveria ativar BOT_A_LED.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(FF),
        "gate_open() deveria ativar FF (fim de fechamento).");
}

void test_gate_open_desativa_bot_b(void) {
    results_set_input("gate_open() deveria desativar BOT_B");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B),
        "gate_open() deveria desativar BOT_B.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B_LED),
        "gate_open() deveria desativar BOT_B_LED.");
}

void test_gate_open_bloqueado_se_ja_abrindo(void) {
    results_set_input("gate_open() chamado enquanto ja esta abrindo");
    gate_open();
    gate_open();
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("nao esta parado"),
        "Segunda chamada deveria registrar warning.");
}

void test_gate_open_inicia_pulso(void) {
    results_set_input("gate_open() deveria iniciar pulso");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PULSE_OPEN_HIGH, gate_get_pulse_state(),
        "gate_open() deveria iniciar pulso em PULSE_OPEN_HIGH.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Fechar Portão
 * ──────────────────────────────────────────────────────────── */

void test_gate_close_ativa_bot_b(void) {
    results_set_input("gate_close()");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(),
        "gate_close() deveria colocar estado em CLOSING.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(BOT_B),
        "gate_close() deveria ativar BOT_B.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(BOT_B_LED),
        "gate_close() deveria ativar BOT_B_LED.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(FA),
        "gate_close() deveria ativar FA (fim de abertura).");
}

void test_gate_close_desativa_bot_a(void) {
    results_set_input("gate_close() deveria desativar BOT_A");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A),
        "gate_close() deveria desativar BOT_A.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A_LED),
        "gate_close() deveria desativar BOT_A_LED.");
}

void test_gate_close_bloqueado_se_ja_fechando(void) {
    results_set_input("gate_close() chamado enquanto ja esta fechando");
    gate_close();
    gate_close();
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("nao esta parado"),
        "Segunda chamada deveria registrar warning.");
}

void test_gate_close_inicia_pulso(void) {
    results_set_input("gate_close() deveria iniciar pulso");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PULSE_CLOSE_HIGH, gate_get_pulse_state(),
        "gate_close() deveria iniciar pulso em PULSE_CLOSE_HIGH.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Parar Portão
 * ──────────────────────────────────────────────────────────── */

void test_gate_stop_zera_tudo(void) {
    results_set_input("gate_stop()");
    gate_open();
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "gate_stop() deveria colocar estado em STOPPED.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A), "BOT_A deve ser zerado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B), "BOT_B deve ser zerado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PULSE_IDLE, gate_get_pulse_state(),
        "gate_stop() deveria zerar pulse_state.");
}

void test_gate_stop_durante_abertura(void) {
    results_set_input("gate_stop() durante abertura");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(), "Deveria estar OPENING.");
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "gate_stop() deveria parar a abertura.");
}

void test_gate_stop_durante_fechamento(void) {
    results_set_input("gate_stop() durante fechamento");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(), "Deveria estar CLOSING.");
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "gate_stop() deveria parar o fechamento.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Máquina de Pulsos
 * ──────────────────────────────────────────────────────────── */

void test_update_pulse_idle_faz_nada(void) {
    results_set_input("gate_update_pulse() com PULSE_IDLE");
    gate_update_pulse();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "update_pulse() com PULSE_IDLE nao deveria mudar estado.");
}

void test_update_pulse_avanca_estados(void) {
    results_set_input("gate_update_pulse() avanca estados corretamente");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PULSE_OPEN_HIGH, gate_get_pulse_state(),
        "Apos gate_open(), deveria estar em PULSE_OPEN_HIGH.");

    /* Simula passagem de tempo para avançar de PULSE_OPEN_HIGH */
    g_gpio_spy.last_micros += PULSE_WIDTH_US;
    gate_update_pulse();

    /* Simula passagem de tempo para avançar de PULSE_OPEN_DONE */
    g_gpio_spy.last_micros += PULSE_WIDTH_US;
    gate_update_pulse();

    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "Apos update_pulse() completo, deveria voltar a STOPPED.");
}

void test_pulso_abertura_completo(void) {
    results_set_input("Ciclo completo de pulso de abertura");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(), "Apos open: OPENING.");
    
    for (int i = 0; i < 10; i++) {
        gate_update_pulse();
    }
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "Apos pulso completo, deveria estar STOPPED.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_A), "BOT_A deveria estar zerado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FF), "FF deveria estar zerado.");
}

void test_pulso_fechamento_completo(void) {
    results_set_input("Ciclo completo de pulso de fechamento");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(), "Apos close: CLOSING.");
    
    for (int i = 0; i < 10; i++) {
        gate_update_pulse();
    }
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "Apos pulso completo, deveria estar STOPPED.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(BOT_B), "BOT_B deveria estar zerado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, gate_get_pin_state(FA), "FA deveria estar zerado.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Limite: Casos Extremos
 * ──────────────────────────────────────────────────────────── */

void test_parada_repetida_nao_causa_problema(void) {
    results_set_input("gate_stop() chamado multiplas vezes");
    gate_open();
    gate_stop();
    gate_stop();
    gate_stop();
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "Multiplos stops deveria manter estado STOPPED.");
}

void test_transicao_rapida_abrir_fechar(void) {
    results_set_input("Abrir -> Stop -> Fechar rapidamente");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(), "Deveria estar OPENING.");
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(), "Deveria estar STOPPED.");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(), "Deveria estar CLOSING.");
}

void test_ambos_pinos_nunca_simultaneos(void) {
    results_set_input("BOT_A e BOT_B nunca devem estar ligados juntos");
    gate_open();
    TEST_ASSERT_TRUE_MESSAGE((gate_get_pin_state(BOT_A) == 1 && gate_get_pin_state(BOT_B) == 0),
        "Em OPENING, apenas BOT_A deve estar ligado.");
    
    gate_stop();
    gate_close();
    TEST_ASSERT_TRUE_MESSAGE((gate_get_pin_state(BOT_A) == 0 && gate_get_pin_state(BOT_B) == 1),
        "Em CLOSING, apenas BOT_B deve estar ligado.");
    
    gate_stop();
    TEST_ASSERT_TRUE_MESSAGE((gate_get_pin_state(BOT_A) == 0 && gate_get_pin_state(BOT_B) == 0),
        "Em STOPPED, ambos devem estar desligados.");
}

void test_leds_acompanham_botoes(void) {
    results_set_input("LEDs devem acompanhar seus botoes");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(gate_get_pin_state(BOT_A), gate_get_pin_state(BOT_A_LED),
        "BOT_A_LED deve acompanhar BOT_A ao abrir.");
    
    gate_stop();
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(gate_get_pin_state(BOT_B), gate_get_pin_state(BOT_B_LED),
        "BOT_B_LED deve acompanhar BOT_B ao fechar.");
}

void test_fims_ligam_durante_movimento(void) {
    results_set_input("Fins de curso devem estar ligados durante movimento");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(FF),
        "FF deveria estar ligado durante abertura.");
    
    gate_stop();
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, gate_get_pin_state(FA),
        "FA deveria estar ligado durante fechamento.");
}

void test_sequencia_complexa_com_multiplos_ciclos(void) {
    results_set_input("Sequencia: abrir->stop->fechar->stop->abrir->pulso->stop");
    
    /* Ciclo 1: Abrir */
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(), "Ciclo 1: OPENING");
    
    /* Parar */
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(), "Ciclo 1: STOPPED");
    
    /* Ciclo 2: Fechar */
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(), "Ciclo 2: CLOSING");
    
    /* Parar */
    gate_stop();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(), "Ciclo 2: STOPPED");
    
    /* Ciclo 3: Abrir com pulso completo */
    gate_open();
    for (int i = 0; i < 10; i++) {
        gate_update_pulse();
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(), "Ciclo 3: STOPPED apos pulso");
}

void test_open_bloqueado_se_ja_fechando(void) {
    results_set_input("gate_open() bloqueado se ja esta CLOSING");
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(), "Deveria estar CLOSING.");
    
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_CLOSING, gate_get_state(),
        "Deveria permanecer CLOSING (abrir bloqueado).");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("nao esta parado"),
        "Log deveria registrar que nao esta parado.");
}

void test_close_bloqueado_se_ja_abrindo(void) {
    results_set_input("gate_close() bloqueado se ja esta OPENING");
    gate_open();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(), "Deveria estar OPENING.");
    
    gate_close();
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_OPENING, gate_get_state(),
        "Deveria permanecer OPENING (fechar bloqueado).");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("nao esta parado"),
        "Log deveria registrar que nao esta parado.");
}

void test_update_pulse_sem_risco_de_overflow(void) {
    results_set_input("update_pulse() chamado muitas vezes sem overflow");
    gate_open();
    for (int i = 0; i < 100; i++) {
        gate_update_pulse();
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(GATE_STOPPED, gate_get_state(),
        "Mesmo com 100 chamadas, deveria voltar a STOPPED.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "motor_fixo.json";

    results_begin("motor_fixo");

    UNITY_BEGIN();
    
    /* Inicialização */
    RUN_TEST(test_gate_init_estado_parado);
    RUN_TEST(test_gate_init_todos_pinos_zerados);
    
    /* Abrir Portão */
    RUN_TEST(test_gate_open_ativa_bot_a);
    RUN_TEST(test_gate_open_desativa_bot_b);
    RUN_TEST(test_gate_open_bloqueado_se_ja_abrindo);
    RUN_TEST(test_gate_open_inicia_pulso);
    
    /* Fechar Portão */
    RUN_TEST(test_gate_close_ativa_bot_b);
    RUN_TEST(test_gate_close_desativa_bot_a);
    RUN_TEST(test_gate_close_bloqueado_se_ja_fechando);
    RUN_TEST(test_gate_close_inicia_pulso);
    
    /* Parar Portão */
    RUN_TEST(test_gate_stop_zera_tudo);
    RUN_TEST(test_gate_stop_durante_abertura);
    RUN_TEST(test_gate_stop_durante_fechamento);
    
    /* Máquina de Pulsos */
    RUN_TEST(test_update_pulse_idle_faz_nada);
    RUN_TEST(test_update_pulse_avanca_estados);
    RUN_TEST(test_pulso_abertura_completo);
    RUN_TEST(test_pulso_fechamento_completo);
    
    /* Limites e Casos Extremos */
    RUN_TEST(test_parada_repetida_nao_causa_problema);
    RUN_TEST(test_transicao_rapida_abrir_fechar);
    RUN_TEST(test_ambos_pinos_nunca_simultaneos);
    RUN_TEST(test_leds_acompanham_botoes);
    RUN_TEST(test_fims_ligam_durante_movimento);
    RUN_TEST(test_sequencia_complexa_com_multiplos_ciclos);
    RUN_TEST(test_open_bloqueado_se_ja_fechando);
    RUN_TEST(test_close_bloqueado_se_ja_abrindo);
    RUN_TEST(test_update_pulse_sem_risco_de_overflow);
    
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
