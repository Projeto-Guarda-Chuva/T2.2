#include <stdio.h>
#include <stdlib.h>
#include "audio_manager.h"
#include "server.h"

#ifndef IS_TEST_ENVIRONMENT
#include <glib.h>
#endif

int main(int argc, char **argv) {
    HttpServer server;

    audio_init();

#ifndef IS_TEST_ENVIRONMENT
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    if (!server_start(&server, 8080, NULL)) {
        fprintf(stderr, "[ERROR] Falha ao iniciar o servidor HTTP.\n");
        return 1;
    }

    printf("[INFO] Servidor rodando na porta 8080. Aguardando JSON via POST...\n");
    g_main_loop_run(loop);

    server_stop(&server);
    g_main_loop_unref(loop);
#else
    server_start(&server, 8080, NULL);
    server_stop(&server);
#endif

    return 0;
}