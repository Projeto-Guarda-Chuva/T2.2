#include "unity.h"
#include "audio_manager.h"
#include "test_log_capture.h"
#include "test_results.h"

void setUp(void) {
    test_log_reset();
}

void tearDown(void) {
    results_record(Unity.CurrentTestName, (int)Unity.CurrentTestFailed);
    for (int i = 0; i < g_test_log.count; i++) {
        results_add_output(g_test_log.entries[i].msg);
    }
}

void test_usar_sem_init_deve_alertar(void) {
    results_set_input("audio_play(\"alerta.mp3\") sem audio_init()");
    audio_play("alerta.mp3");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('W') > 0 || test_log_count_level('E') > 0,
        "Usar o componente antes de audio_init() deveria emitir um alerta de 'nao inicializado', mas nenhum foi registrado.");
}

void test_init_funciona(void) {
    results_set_input("audio_init()");
    audio_init();
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, test_log_count_level('I'),
        "audio_init() deveria emitir exatamente 3 mensagens INFO.");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("100"),
        "audio_init() deveria reportar o volume inicial de 100%.");
}

void test_play_com_arquivo_valido(void) {
    results_set_input("audio_play(\"alerta.mp3\")");
    audio_play("alerta.mp3");
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("alerta.mp3"),
        "audio_play deveria registrar o nome do arquivo reproduzido.");
    TEST_ASSERT_TRUE_MESSAGE(audio_is_playing(),
        "apos audio_play com arquivo valido, o estado deveria ser 'reproduzindo'.");
}

void test_play_arquivo_nulo_rejeitado(void) {
    results_set_input("audio_play(NULL)");
    audio_stop();
    test_log_reset();
    audio_play(NULL);
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "audio_play(NULL) deveria registrar erro de arquivo invalido.");
    TEST_ASSERT_FALSE_MESSAGE(audio_is_playing(),
        "audio_play(NULL) nao deveria iniciar a reproducao.");
}

void test_play_arquivo_vazio_rejeitado(void) {
    results_set_input("audio_play(\"\")");
    audio_stop();
    test_log_reset();
    audio_play("");
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('E') > 0,
        "audio_play(\"\") deveria registrar erro de arquivo invalido.");
    TEST_ASSERT_FALSE_MESSAGE(audio_is_playing(),
        "audio_play(\"\") nao deveria iniciar a reproducao.");
}

void test_play_default(void) {
    results_set_input("audio_play_default()");
    audio_play_default();
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("default.mp3"),
        "audio_play_default deveria reproduzir o arquivo default.mp3.");
    TEST_ASSERT_TRUE_MESSAGE(audio_is_playing(),
        "audio_play_default deveria iniciar a reproducao.");
}

void test_is_playing_reflete_estado(void) {
    results_set_input("audio_play(\"x.mp3\") depois audio_stop()");
    audio_play("x.mp3");
    TEST_ASSERT_TRUE_MESSAGE(audio_is_playing(),
        "apos audio_play, audio_is_playing deveria ser verdadeiro.");
    audio_stop();
    TEST_ASSERT_FALSE_MESSAGE(audio_is_playing(),
        "apos audio_stop, audio_is_playing deveria ser falso.");
}

void test_stop_registra_parada(void) {
    results_set_input("audio_play(\"x.mp3\"); audio_stop()");
    audio_play("x.mp3");
    test_log_reset();
    audio_stop();
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("Parando"),
        "audio_stop deveria registrar a parada da reproducao.");
}

void test_stop_sem_reproduzir_deveria_avisar(void) {
    results_set_input("audio_stop() sem nada em reproducao");
    audio_stop();
    test_log_reset();
    audio_stop();
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('W') > 0,
        "Parar sem nada em reproducao deveria emitir aviso; a condicao 'if(AUDIO_STOPPED)' e sempre falsa e o aviso nunca e alcancado.");
}

void test_volume_persiste(void) {
    results_set_input("audio_set_volume(50); audio_set_volume(70)");
    audio_set_volume(50);
    test_log_reset();
    audio_set_volume(70);
    TEST_ASSERT_TRUE_MESSAGE(test_log_contains("de 50 para 70"),
        "O volume ajustado deveria ser armazenado (esperado 'de 50 para 70').");
}

void test_volume_maximo_valido_sem_alerta(void) {
    results_set_input("audio_set_volume(100)");
    audio_set_volume(100);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, test_log_count_level('W'),
        "Volume 100 e valido: nao deveria emitir alerta.");
}

void test_volume_minimo_sem_alerta(void) {
    results_set_input("audio_set_volume(0)");
    audio_set_volume(0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, test_log_count_level('W'),
        "Volume 0 e valido: nao deveria emitir alerta.");
}

void test_volume_acima_do_limite_alerta(void) {
    results_set_input("audio_set_volume(101)");
    audio_set_volume(101);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, test_log_count_level('W'),
        "Volume 101 e invalido: deveria emitir exatamente 1 alerta.");
}

void test_volume_clamp_reflete_na_mensagem(void) {
    results_set_input("audio_set_volume(200)");
    audio_set_volume(200);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, test_log_count_level('W'),
        "Volume 200 e invalido: deveria emitir alerta.");
    TEST_ASSERT_FALSE_MESSAGE(test_log_contains("para 200"),
        "Apos cortar o volume para 100, a mensagem de troca nao deveria exibir o valor original 200.");
}

void test_volume_negativo_vira_255_e_corta(void) {
    results_set_input("audio_set_volume(-1)");
    audio_set_volume(-1);
    TEST_ASSERT_TRUE_MESSAGE(test_log_count_level('W') > 0,
        "Volume -1 vira 255 no uint8_t e deveria ser tratado como invalido e cortado para 100.");
}

int main(int argc, char **argv) {
    const char *out = (argc > 1) ? argv[1] : "audio.json";

    results_begin("audio");

    UNITY_BEGIN();
    RUN_TEST(test_usar_sem_init_deve_alertar);
    RUN_TEST(test_init_funciona);
    RUN_TEST(test_play_com_arquivo_valido);
    RUN_TEST(test_play_arquivo_nulo_rejeitado);
    RUN_TEST(test_play_arquivo_vazio_rejeitado);
    RUN_TEST(test_play_default);
    RUN_TEST(test_is_playing_reflete_estado);
    RUN_TEST(test_stop_registra_parada);
    RUN_TEST(test_stop_sem_reproduzir_deveria_avisar);
    RUN_TEST(test_volume_persiste);
    RUN_TEST(test_volume_maximo_valido_sem_alerta);
    RUN_TEST(test_volume_minimo_sem_alerta);
    RUN_TEST(test_volume_acima_do_limite_alerta);
    RUN_TEST(test_volume_clamp_reflete_na_mensagem);
    RUN_TEST(test_volume_negativo_vira_255_e_corta);
    int rc = UNITY_END();

    results_write(out);
    return rc;
}
