#include <glib.h>
#include <stdio.h>
#include "audio_manager.h"
#include "server.h"

int main(int argc, char *argv[]) {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    AudioManager am;
    HttpServer server;

    g_print("Inicializando Atuador de Audio...\n");
    audio_manager_init(&am, loop);

    if (!server_start(&server, 8080, &am)) {
        g_printerr("Falha ao iniciar o servidor HTTP.\n");
        audio_manager_cleanup(&am);
        g_main_loop_unref(loop);
        return 1;
    }

    g_print("Servidor HTTP rodando na porta 8080. Aguardando requisicoes...\n");
    
    g_main_loop_run(loop);

    server_stop(&server);
    audio_manager_cleanup(&am);
    g_main_loop_unref(loop);

    return 0;
}