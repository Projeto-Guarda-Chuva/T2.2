#include "fake_wifi.h"

wifi_spy_t g_wifi_spy = {0};

void wifi_spy_reset(void)
{
    g_wifi_spy.is_connected = 0;
    g_wifi_spy.wifi_begin_calls = 0;
    g_wifi_spy.connect_attempts = 0;
}

void wifi_spy_set_connected(int connected)
{
    g_wifi_spy.is_connected = connected;
}

int WiFi_status(void)
{
    return g_wifi_spy.is_connected ? WL_CONNECTED : WL_DISCONNECTED;
}

void WiFi_begin(const char *ssid)
{
    (void)ssid;
    g_wifi_spy.wifi_begin_calls++;
}

void WiFi_mode(int mode)
{
    (void)mode;
}
