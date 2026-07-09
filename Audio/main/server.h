#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include "audio_manager.h"

#ifndef PRODUCTION_ENV
    #define IS_TEST_ENVIRONMENT
#endif

#ifndef IS_TEST_ENVIRONMENT
#include <microhttpd.h>
typedef struct {
    struct MHD_Daemon *http_daemon;
    AudioManager *am;
} HttpServer;
#else
typedef struct {
    void *http_daemon;
    void *am;
} HttpServer;
#endif

bool server_start(HttpServer *server, int port, AudioManager *am);

void server_stop(HttpServer *server);

#endif