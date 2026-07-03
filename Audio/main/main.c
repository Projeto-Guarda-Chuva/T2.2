#include <stdio.h>

#include "audio_manager.h"


int main(void) {
    printf("========================================\n");
    printf("   Teste do Atuador de Áudio\n");
    printf("========================================\n\n");

    if (!audio_init()) {
        printf("Erro ao inicializar o atuador!\n");
        printf("Motivo: %s\n", audio_get_last_error());
        return 1;
    }

    printf("Atuador inicializado com sucesso!\n");

    if (!audio_set_volume(70)) {
        printf("Erro ao definir volume!\n");
        printf("%s\n", audio_get_last_error());
    }

    printf("Volume atual: %u%%\n", audio_get_volume());

    printf("\nIniciando reprodução...\n");

    if (!audio_play("../audios/alerta.mp3")) {
        printf("Erro ao reproduzir!\n");
        printf("%s\n", audio_get_last_error());

        audio_deinit();
        return 1;
    }

    printf("Arquivo atual: %s\n", audio_get_current_file());

    printf("\nPressione ENTER para parar o áudio...\n");

    getchar();

    audio_stop();

    printf("Áudio parado.\n");

    audio_deinit();

    printf("Recursos liberados.\n");

    return 0;
}
