#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PRODUCTION_ENV
    #define IS_TEST_ENVIRONMENT
#endif

#ifndef IS_TEST_ENVIRONMENT
#include <microhttpd.h>
#include <json-c/json.h>
#include <glib.h>

#define POSTBUFFERSIZE 512

struct connection_info {
    char *data;
    size_t size;
};

static void handle_audio_command(const char *json_str) {
    struct json_object *parsed_json = json_tokener_parse(json_str);
    if (!parsed_json) {
        g_printerr("Erro ao parsear JSON recebido!\n");
        return;
    }

    struct json_object *id_obj;
    if (json_object_object_get_ex(parsed_json, "id", &id_obj)) {
        int id = json_object_get_int(id_obj);
        
        switch (id) {
            case 40:
                audio_stop();
                break;

            case 41: {
                struct json_object *file_obj;
                const char *file_name = NULL;
                if (json_object_object_get_ex(parsed_json, "file", &file_obj)) {
                    file_name = json_object_get_string(file_obj);
                }
                audio_play(file_name);
                break;
            }

            case 42: {
                struct json_object *vol_obj;
                if (json_object_object_get_ex(parsed_json, "volume", &vol_obj)) {
                    int volume = json_object_get_int(vol_obj);
                    audio_set_volume((int8_t)volume);
                }
                break;
            }

            default:
                g_print("ID de comando desconhecido: %d\n!", id);
                break;
        }
    }
    json_object_put(parsed_json);
}


static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    if (strcmp(url, "/audio") != 0 || strcmp(method, "POST") != 0) {
        struct MHD_Response *response = MHD_create_response_from_buffer(17, "Endpoint invalido", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (*con_cls == NULL) {
        struct connection_info *con_info = malloc(sizeof(struct connection_info));
        con_info->data = NULL;
        con_info->size = 0;
        *con_cls = (void *)con_info;
        return MHD_YES;
    }

    struct connection_info *con_info = (struct connection_info *)*con_cls;

    if (*upload_data_size != 0) {
        con_info->data = realloc(con_info->data, con_info->size + *upload_data_size + 1);
        memcpy(con_info->data + con_info->size, upload_data, *upload_data_size);
        con_info->size += *upload_data_size;
        con_info->data[con_info->size] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

    if (con_info->data != NULL) {
        handle_audio_command(con_info->data);
        free(con_info->data);
    }

    free(con_info);

    const char *reply_str = "OK\n";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(reply_str), (void *)reply_str, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "text/plain");
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}
#endif


bool server_start(HttpServer *server, int port, AudioManager *am) {
    server->am = am;
#ifndef IS_TEST_ENVIRONMENT
    server->http_daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL,
                                      &answer_to_connection, server, MHD_OPTION_END);
    return (server->http_daemon != NULL);
#else
    printf("[INFO] Mock Server: Servidor HTTP simulado com sucesso na porta %d.\n", port);
    return true;
#endif
}


void server_stop(HttpServer *server) {
#ifndef IS_TEST_ENVIRONMENT
    if (server->http_daemon) {
        MHD_stop_daemon(server->http_daemon);
    }
#else
    printf("[INFO] Mock Server: Servidor HTTP simulado finalizado.\n");
#endif
}