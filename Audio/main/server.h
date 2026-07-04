#ifndef SERVER_H
#define SERVER_H

#include <microhttpd.h>
#include "audio_manager.h"

typedef struct {
    struct MHD_Daemon *http_daemon;
    AudioManager *am;
} HttpServer;

bool server_start(HttpServer *server, int port, AudioManager *am);

void server_stop(HttpServer *server);

#endif