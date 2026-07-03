#include "fake_arduino.h"

motor_spy_t g_motor_spy = {0};

void motor_spy_reset(void)
{
    g_motor_spy.rpwm_value = 0;
    g_motor_spy.lpwm_value = 0;
    g_motor_spy.setup_calls = 0;
    for (int i = 0; i < 40; i++) {
        g_motor_spy.pin_states[i] = -1;
    }
}

void pinMode(int pin, int mode)
{
    (void)pin;
    (void)mode;
}

void digitalWrite(int pin, int value)
{
    if (pin >= 0 && pin < 40) {
        g_motor_spy.pin_states[pin] = value;
    }
}

int digitalRead(int pin)
{
    if (pin >= 0 && pin < 40) {
        return g_motor_spy.pin_states[pin];
    }
    return 0;
}

void ledcSetup(int channel, int freq, int resolution)
{
    (void)channel;
    (void)freq;
    (void)resolution;
    g_motor_spy.setup_calls++;
}

void ledcAttachPin(int pin, int channel)
{
    (void)pin;
    (void)channel;
}

void ledcWrite(int channel, int value)
{
    if (channel == 0) {  /* CANAL_RPWM */
        g_motor_spy.rpwm_value = value;
    } else if (channel == 1) {  /* CANAL_LPWM */
        g_motor_spy.lpwm_value = value;
    }
}
