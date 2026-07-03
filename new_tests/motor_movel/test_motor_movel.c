#include "unity.h"
#include "motor_movel.h"
#include "mocks/fake_arduino.h"
#include "mocks/test_log_capture.h"
#include "../support/test_results.h"

void setUp(void) {
    motor_spy_reset();
    test_log_reset();
    motor_init();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);

    char line[192];
    snprintf(line, sizeof(line),
             "rpwm=%d lpwm=%d fcs=%d fci=%d state=%d",
             g_motor_spy.rpwm_value, g_motor_spy.lpwm_value,
             fim_superior(), fim_inferior(), motor_get_state());
    results_add_output(line);

    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

/* ────────────────────────────────────────────────────────────
 * Testes de Inicialização
 * ──────────────────────────────────────────────────────────── */

void test_init_funciona(void) {
    results_set_input("motor_init()");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "motor_init() deveria deixar o motor no estado PARADO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "motor_init() deveria ter RPWM em 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "motor_init() deveria ter LPWM em 0.");
}

void test_enable_pin_inicializa_high(void) {
    results_set_input("motor_init() deve habilitar o driver");
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, g_motor_spy.pin_states[ENABLE_PIN],
        "motor_init() deveria colocar ENABLE_PIN em HIGH.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Comando: PARAR
 * ──────────────────────────────────────────────────────────── */

void test_motor_parar_zera_pwm(void) {
    results_set_input("motor_parar()");
    motor_parar();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "motor_parar() deveria zerar RPWM.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "motor_parar() deveria zerar LPWM.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "motor_parar() deveria deixar o motor em PARADO.");
}

void test_parar_desde_subindo(void) {
    results_set_input("motor_subir() depois motor_parar()");
    motor_subir();
    motor_parar();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "Apos motor_parar(), RPWM deveria ser 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Apos motor_parar(), estado deveria ser PARADO.");
}

void test_parar_desde_descendo(void) {
    results_set_input("motor_descer() depois motor_parar()");
    motor_descer();
    motor_parar();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "Apos motor_parar(), LPWM deveria ser 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Apos motor_parar(), estado deveria ser PARADO.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Comando: SUBIR
 * ──────────────────────────────────────────────────────────── */

void test_motor_subir_ativa_rpwm(void) {
    results_set_input("motor_subir()");
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value,
        "motor_subir() deveria colocar RPWM em VELOCIDADE.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "motor_subir() deveria zerar LPWM.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(),
        "motor_subir() deveria colocar estado em SUBINDO.");
}

void test_subir_repetido_nao_muda(void) {
    results_set_input("motor_subir() chamado duas vezes");
    motor_subir();
    int rpwm_1 = g_motor_spy.rpwm_value;
    motor_subir();
    int rpwm_2 = g_motor_spy.rpwm_value;
    TEST_ASSERT_EQUAL_INT_MESSAGE(rpwm_1, rpwm_2,
        "motor_subir() repetido deveria manter o mesmo PWM.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("ja esta subindo"),
        "motor_subir() repetido deveria registrar warning.");
}

void test_subir_bloqueado_por_fcs(void) {
    results_set_input("motor_subir() com FCS acionado");
    motor_set_fcs(1);  /* FCS acionado */
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "motor_subir() deveria ser bloqueado se FCS ja esta acionado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria permanecer PARADO se FCS esta acionado.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "motor_subir() com FCS acionado deveria registrar erro.");
}

void test_subir_desde_descendo_para_primeiro(void) {
    results_set_input("motor_descer() depois motor_subir()");
    motor_descer();
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(),
        "Apos motor_subir(), estado deveria ser SUBINDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value,
        "Apos motor_subir(), RPWM deveria estar ativo.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "Apos motor_subir(), LPWM deveria estar zerado.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Comando: DESCER
 * ──────────────────────────────────────────────────────────── */

void test_motor_descer_ativa_lpwm(void) {
    results_set_input("motor_descer()");
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.lpwm_value,
        "motor_descer() deveria colocar LPWM em VELOCIDADE.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "motor_descer() deveria zerar RPWM.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(),
        "motor_descer() deveria colocar estado em DESCENDO.");
}

void test_descer_repetido_nao_muda(void) {
    results_set_input("motor_descer() chamado duas vezes");
    motor_descer();
    int lpwm_1 = g_motor_spy.lpwm_value;
    motor_descer();
    int lpwm_2 = g_motor_spy.lpwm_value;
    TEST_ASSERT_EQUAL_INT_MESSAGE(lpwm_1, lpwm_2,
        "motor_descer() repetido deveria manter o mesmo PWM.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("ja esta descendo"),
        "motor_descer() repetido deveria registrar warning.");
}

void test_descer_bloqueado_por_fci(void) {
    results_set_input("motor_descer() com FCI acionado");
    motor_set_fci(1);  /* FCI acionado */
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "motor_descer() deveria ser bloqueado se FCI ja esta acionado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria permanecer PARADO se FCI esta acionado.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "motor_descer() com FCI acionado deveria registrar erro.");
}

void test_descer_desde_subindo_para_primeiro(void) {
    results_set_input("motor_subir() depois motor_descer()");
    motor_subir();
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(),
        "Apos motor_descer(), estado deveria ser DESCENDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.lpwm_value,
        "Apos motor_descer(), LPWM deveria estar ativo.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "Apos motor_descer(), RPWM deveria estar zerado.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Fins de Curso
 * ──────────────────────────────────────────────────────────── */

void test_fim_superior_nao_acionado_por_padrao(void) {
    results_set_input("fim_superior() ao iniciar");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fim_superior(),
        "fim_superior() deveria retornar 0 ao iniciar.");
}

void test_fim_inferior_nao_acionado_por_padrao(void) {
    results_set_input("fim_inferior() ao iniciar");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fim_inferior(),
        "fim_inferior() deveria retornar 0 ao iniciar.");
}

void test_fim_superior_pode_ser_acionado(void) {
    results_set_input("motor_set_fcs(1)");
    motor_set_fcs(1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, fim_superior(),
        "Apos motor_set_fcs(1), fim_superior() deveria retornar 1.");
}

void test_fim_inferior_pode_ser_acionado(void) {
    results_set_input("motor_set_fci(1)");
    motor_set_fci(1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, fim_inferior(),
        "Apos motor_set_fci(1), fim_inferior() deveria retornar 1.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Verificação de Fins de Curso
 * ──────────────────────────────────────────────────────────── */

void test_verificar_fcs_para_motor_subindo(void) {
    results_set_input("motor_subir() depois motor_set_fcs(1) e verificar_fins_de_curso()");
    motor_subir();
    motor_set_fcs(1);  /* FCS acionado */
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "verificar_fins_de_curso() deveria parar motor que estava SUBINDO com FCS acionado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "Apos parada por FCS, RPWM deveria ser 0.");
}

void test_verificar_fci_para_motor_descendo(void) {
    results_set_input("motor_descer() depois motor_set_fci(1) e verificar_fins_de_curso()");
    motor_descer();
    motor_set_fci(1);  /* FCI acionado */
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "verificar_fins_de_curso() deveria parar motor que estava DESCENDO com FCI acionado.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "Apos parada por FCI, LPWM deveria ser 0.");
}

void test_verificar_fcs_nao_afeta_motor_parado(void) {
    results_set_input("motor_parar() depois motor_set_fcs(1) e verificar_fins_de_curso()");
    motor_parar();
    motor_set_fcs(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "verificar_fins_de_curso() nao deveria afetar motor PARADO.");
}

void test_verificar_fci_nao_afeta_motor_parado(void) {
    results_set_input("motor_parar() depois motor_set_fci(1) e verificar_fins_de_curso()");
    motor_parar();
    motor_set_fci(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "verificar_fins_de_curso() nao deveria afetar motor PARADO.");
}

void test_verificar_fcs_nao_afeta_motor_descendo(void) {
    results_set_input("motor_descer() depois motor_set_fcs(1) e verificar_fins_de_curso()");
    motor_descer();
    motor_set_fcs(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(),
        "verificar_fins_de_curso() nao deveria parar motor DESCENDO se FCS esta acionado (apenas FCI).");
}

void test_verificar_fci_nao_afeta_motor_subindo(void) {
    results_set_input("motor_subir() depois motor_set_fci(1) e verificar_fins_de_curso()");
    motor_subir();
    motor_set_fci(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(),
        "verificar_fins_de_curso() nao deveria parar motor SUBINDO se FCI esta acionado (apenas FCS).");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Segurança: Transições de Estado
 * ──────────────────────────────────────────────────────────── */

void test_alternancia_segura_subir_descer(void) {
    results_set_input("motor_subir() -> motor_descer() com transicoes seguras");
    
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Deveria estar SUBINDO.");
    
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(), "Deveria estar DESCENDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.lpwm_value, "LPWM deveria estar ativo.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value, "RPWM deveria estar zerado.");
}

void test_alternancia_segura_descer_subir(void) {
    results_set_input("motor_descer() -> motor_subir() com transicoes seguras");
    
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(), "Deveria estar DESCENDO.");
    
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Deveria estar SUBINDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value, "RPWM deveria estar ativo.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value, "LPWM deveria estar zerado.");
}

void test_velocidade_constante(void) {
    results_set_input("Verificar que VELOCIDADE eh constante");
    motor_subir();
    int velocidade_subir = g_motor_spy.rpwm_value;
    motor_descer();
    int velocidade_descer = g_motor_spy.lpwm_value;
    TEST_ASSERT_EQUAL_INT_MESSAGE(velocidade_subir, velocidade_descer,
        "Velocidade ao subir deveria ser igual a velocidade ao descer.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, velocidade_subir,
        "Velocidade deveria ser VELOCIDADE.");
}

/* ────────────────────────────────────────────────────────────
 * Testes de Limite: Casos Extremos
 * ──────────────────────────────────────────────────────────── */

void test_ambos_fins_acionados_simultaneamente(void) {
    results_set_input("Acionar FCS e FCI simultaneamente");
    motor_subir();
    motor_set_fcs(1);  /* FCS acionado */
    motor_set_fci(1);  /* FCI acionado */
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria parar se ambos fins de curso estao acionados.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("FCS acionado por hardware"),
        "Log deveria registrar o evento de FCS.");
}

void test_transicao_rapida_subir_descer_subir(void) {
    results_set_input("Transicao rapida: subir->descer->subir");
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Deveria estar SUBINDO.");
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(), "Deveria estar DESCENDO.");
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Deveria estar SUBINDO novamente.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value,
        "RPWM deveria estar ativo.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "LPWM deveria estar zerado.");
}

void test_parada_repetida_nao_causa_problema(void) {
    results_set_input("motor_parar() chamado multiplas vezes consecutivas");
    motor_subir();
    motor_parar();
    motor_parar();
    motor_parar();
    motor_parar();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria continuar PARADO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "RPWM deveria ser 0.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "LPWM deveria ser 0.");
}

void test_fim_curso_durante_movimento_subindo(void) {
    results_set_input("FCS acionado enquanto motor está SUBINDO");
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Deveria estar SUBINDO.");
    motor_set_fcs(1);  /* Simula FCS acionado durante movimento */
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria parar imediatamente ao detectar FCS.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.rpwm_value,
        "RPWM deveria ser zerado.");
}

void test_fim_curso_durante_movimento_descendo(void) {
    results_set_input("FCI acionado enquanto motor está DESCENDO");
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(), "Deveria estar DESCENDO.");
    motor_set_fci(1);  /* Simula FCI acionado durante movimento */
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria parar imediatamente ao detectar FCI.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_motor_spy.lpwm_value,
        "LPWM deveria ser zerado.");
}

void test_liberar_e_reaplicar_fim_curso(void) {
    results_set_input("Acionar FCS, liberar, e tentar subir novamente");
    motor_set_fcs(1);  /* FCS acionado */
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor nao deveria subir com FCS acionado.");
    
    motor_set_fcs(0);  /* FCS liberado */
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(),
        "Motor deveria subir apos liberar FCS.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value,
        "RPWM deveria estar ativo.");
}

void test_ambos_pwm_nunca_ligados_simultaneamente(void) {
    results_set_input("Verificar que RPWM e LPWM nunca estao ligados juntos");
    motor_subir();
    TEST_ASSERT_TRUE_MESSAGE((g_motor_spy.rpwm_value > 0 && g_motor_spy.lpwm_value == 0),
        "Em SUBINDO, apenas RPWM deve estar ligado.");
    
    motor_descer();
    TEST_ASSERT_TRUE_MESSAGE((g_motor_spy.rpwm_value == 0 && g_motor_spy.lpwm_value > 0),
        "Em DESCENDO, apenas LPWM deve estar ligado.");
    
    motor_parar();
    TEST_ASSERT_TRUE_MESSAGE((g_motor_spy.rpwm_value == 0 && g_motor_spy.lpwm_value == 0),
        "Em PARADO, ambos PWMs devem estar desligados.");
}

void test_mudanca_fcs_nao_afeta_descendo(void) {
    results_set_input("motor_descer() e depois acionar FCS");
    motor_descer();
    motor_set_fcs(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(),
        "FCS nao deveria afetar motor DESCENDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.lpwm_value,
        "LPWM deveria continuar em VELOCIDADE.");
}

void test_mudanca_fci_nao_afeta_subindo(void) {
    results_set_input("motor_subir() e depois acionar FCI");
    motor_subir();
    motor_set_fci(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(),
        "FCI nao deveria afetar motor SUBINDO.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(VELOCIDADE, g_motor_spy.rpwm_value,
        "RPWM deveria continuar em VELOCIDADE.");
}

void test_sequencia_complexa_com_limites(void) {
    results_set_input("Sequencia complexa: subir->parar->descer->fci->parar->subir->fcs");
    
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Passo 1: SUBINDO");
    
    motor_parar();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(), "Passo 2: PARADO");
    
    motor_descer();
    TEST_ASSERT_EQUAL_INT_MESSAGE(DESCENDO, motor_get_state(), "Passo 3: DESCENDO");
    
    motor_set_fci(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(), "Passo 4: PARADO por FCI");
    
    motor_set_fci(0);
    motor_subir();
    TEST_ASSERT_EQUAL_INT_MESSAGE(SUBINDO, motor_get_state(), "Passo 5: SUBINDO");
    
    motor_set_fcs(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(), "Passo 6: PARADO por FCS");
    
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('I') >= 8,
        "Deveria haver pelo menos 8 mensagens INFO registradas.");
}

void test_descer_repetido_com_fci_acionado_no_meio(void) {
    results_set_input("motor_descer(), depois motor_descer(), depois acionar FCI");
    motor_descer();
    motor_descer();  /* Chamada repetida - deveria ser ignorada */
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("ja esta descendo"),
        "Segunda chamada deveria registrar warning.");
    
    motor_set_fci(1);
    verificar_fins_de_curso();
    TEST_ASSERT_EQUAL_INT_MESSAGE(PARADO, motor_get_state(),
        "Motor deveria parar ao detectar FCI.");
}

void test_enable_pin_permanece_high(void) {
    results_set_input("ENABLE_PIN deveria permanecer HIGH durante operacao");
    int enable_antes = g_motor_spy.pin_states[ENABLE_PIN];
    motor_subir();
    int enable_subir = g_motor_spy.pin_states[ENABLE_PIN];
    motor_descer();
    int enable_descer = g_motor_spy.pin_states[ENABLE_PIN];
    motor_parar();
    int enable_depois = g_motor_spy.pin_states[ENABLE_PIN];
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, enable_antes, "ENABLE deve ser HIGH inicialmente.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, enable_subir, "ENABLE deve ser HIGH ao subir.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, enable_descer, "ENABLE deve ser HIGH ao descer.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, enable_depois, "ENABLE deve ser HIGH ao parar.");
}

void test_fcs_fci_desligamento_duplicado(void) {
    results_set_input("Desligar FCS/FCI multiplas vezes");
    motor_set_fcs(1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, fim_superior(), "FCS deveria estar acionado.");
    motor_set_fcs(0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fim_superior(), "FCS deveria estar desacionado.");
    motor_set_fcs(0);  /* Chamada duplicada */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fim_superior(), "FCS deveria continuar desacionado.");
    motor_set_fcs(0);  /* Mais uma */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fim_superior(), "FCS deveria permanecer desacionado.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "motor_movel.json";

    results_begin("motor_movel");

    UNITY_BEGIN();
    
    /* Inicialização */
    RUN_TEST(test_init_funciona);
    RUN_TEST(test_enable_pin_inicializa_high);
    
    /* Comando: PARAR */
    RUN_TEST(test_motor_parar_zera_pwm);
    RUN_TEST(test_parar_desde_subindo);
    RUN_TEST(test_parar_desde_descendo);
    
    /* Comando: SUBIR */
    RUN_TEST(test_motor_subir_ativa_rpwm);
    RUN_TEST(test_subir_repetido_nao_muda);
    RUN_TEST(test_subir_bloqueado_por_fcs);
    RUN_TEST(test_subir_desde_descendo_para_primeiro);
    
    /* Comando: DESCER */
    RUN_TEST(test_motor_descer_ativa_lpwm);
    RUN_TEST(test_descer_repetido_nao_muda);
    RUN_TEST(test_descer_bloqueado_por_fci);
    RUN_TEST(test_descer_desde_subindo_para_primeiro);
    
    /* Fins de Curso */
    RUN_TEST(test_fim_superior_nao_acionado_por_padrao);
    RUN_TEST(test_fim_inferior_nao_acionado_por_padrao);
    RUN_TEST(test_fim_superior_pode_ser_acionado);
    RUN_TEST(test_fim_inferior_pode_ser_acionado);
    
    /* Verificação de Fins de Curso */
    RUN_TEST(test_verificar_fcs_para_motor_subindo);
    RUN_TEST(test_verificar_fci_para_motor_descendo);
    RUN_TEST(test_verificar_fcs_nao_afeta_motor_parado);
    RUN_TEST(test_verificar_fci_nao_afeta_motor_parado);
    RUN_TEST(test_verificar_fcs_nao_afeta_motor_descendo);
    RUN_TEST(test_verificar_fci_nao_afeta_motor_subindo);
    
    /* Segurança: Transições */
    RUN_TEST(test_alternancia_segura_subir_descer);
    RUN_TEST(test_alternancia_segura_descer_subir);
    RUN_TEST(test_velocidade_constante);
    
    /* Testes de Limite: Casos Extremos */
    RUN_TEST(test_ambos_fins_acionados_simultaneamente);
    RUN_TEST(test_transicao_rapida_subir_descer_subir);
    RUN_TEST(test_parada_repetida_nao_causa_problema);
    RUN_TEST(test_fim_curso_durante_movimento_subindo);
    RUN_TEST(test_fim_curso_durante_movimento_descendo);
    RUN_TEST(test_liberar_e_reaplicar_fim_curso);
    RUN_TEST(test_ambos_pwm_nunca_ligados_simultaneamente);
    RUN_TEST(test_mudanca_fcs_nao_afeta_descendo);
    RUN_TEST(test_mudanca_fci_nao_afeta_subindo);
    RUN_TEST(test_sequencia_complexa_com_limites);
    RUN_TEST(test_descer_repetido_com_fci_acionado_no_meio);
    RUN_TEST(test_enable_pin_permanece_high);
    RUN_TEST(test_fcs_fci_desligamento_duplicado);
    
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
