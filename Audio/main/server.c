#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#define POSTBUFFERSIZE 512

struct connection_info {
    char *data;
    size_t size;
};

// Processa o JSON após receber todo o corpo do POST:
static void handle_audio_command(AudioManager *am, const char *json_str) {
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
                audio_manager_stop(am);
                break;

            case 41: {
                struct json_object *file_obj;
                const char *file_name = NULL;
                if (json_object_object_get_ex(parsed_json, "file", &file_obj)) {
                    file_name = json_object_get_string(file_obj);
                }

                audio_manager_start_playlist(am, file_name);
                break;
            }

            case 42: {
                struct json_object *vol_obj;
                if (json_object_object_get_ex(parsed_json, "volume", &vol_obj)) {
                    int volume = json_object_get_int(vol_obj);
                    audio_manager_set_volume(am, volume);
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


static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                           const char *url, const char *method,
                                           const char *version, const char *upload_data,
                                           size_t *upload_data_size, void **con_cls) {
    HttpServer *server = (HttpServer *)cls;
    if (strcmp(url, "/audio") != 0 || strcmp(method, "POST") != 0) {
        struct MHD_Response *response = MHD_create_response_from_buffer(17, "Endpoint invalido", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Primeira chamada da requisição: aloca o buffer para receber o JSON:
    if (*con_cls == NULL) {
        struct connection_info *con_info = malloc(sizeof(struct connection_info));
        con_info->data = NULL;
        con_info->size = 0;
        *con_cls = (void *)con_info;
        return MHD_YES;
    }

    struct connection_info *con_info = (struct connection_info *)*con_cls;

    // Se ainda houver dados chegando no POST, acumula no buffer:
    if (*upload_data_size != 0) {
        con_info->data = realloc(con_info->data, con_info->size + *upload_data_size + 1);
        memcpy(con_info->data + con_info->size, upload_data, *upload_data_size);
        con_info->size += *upload_data_size;
        con_info->data[con_info->size] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

    // Todo o JSON foi recebido:
    if (con_info->data != NULL) {
        handle_audio_command(server->am, con_info->data);
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


bool server_start(HttpServer *server, int port, AudioManager *am) {
    server->am = am;
    server->http_daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL,
                                      &answer_to_connection, server, MHD_OPTION_END);
    return (server->http_daemon != NULL);
}


void server_stop(HttpServer *server) {
    if (server->http_daemon) {
        MHD_stop_daemon(server->http_daemon);
    }
}
