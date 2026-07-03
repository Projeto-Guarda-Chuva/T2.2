#pragma once

/* WiFi Macros */
#define WL_CONNECTED    3
#define WL_DISCONNECTED 0

/* ── State espionagem ─────────────────────────────────── */
typedef struct {
    int is_connected;
    int wifi_begin_calls;
    int connect_attempts;
} wifi_spy_t;

extern wifi_spy_t g_wifi_spy;

/* ── WiFi status ─────────────────────────────────────── */
int WiFi_status(void);
void WiFi_begin(const char *ssid);
void WiFi_mode(int mode);

/* ── Funções de espionagem ─────────────────────────────── */
void wifi_spy_reset(void);
void wifi_spy_set_connected(int connected);
