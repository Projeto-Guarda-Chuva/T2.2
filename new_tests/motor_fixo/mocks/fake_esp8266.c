#include "fake_esp8266.h"

gpio_spy_t g_gpio_spy = {0};

static int interrupts_disabled = 0;
static unsigned long micros_counter = 0;

void gpio_spy_reset(void)
{
    for (int i = 0; i < 20; i++) {
        g_gpio_spy.pin_states[i] = 0;
    }
    g_gpio_spy.micros_calls = 0;
    g_gpio_spy.last_micros = 0;
    micros_counter = 0;
    interrupts_disabled = 0;
}

int gpio_read_pin(int pin)
{
    if (pin >= 0 && pin < 20) {
        return g_gpio_spy.pin_states[pin];
    }
    return 0;
}

void gpio_set_pin(int pin, int value)
{
    if (pin >= 0 && pin < 20) {
        g_gpio_spy.pin_states[pin] = value;
    }
}

void pinMode(int pin, int mode)
{
    (void)pin;
    (void)mode;
}

void digitalWrite(int pin, int value)
{
    gpio_set_pin(pin, value);
}

int digitalRead(int pin)
{
    return gpio_read_pin(pin);
}

void noInterrupts(void)
{
    interrupts_disabled = 1;
}

void interrupts(void)
{
    interrupts_disabled = 0;
}

unsigned long micros(void)
{
    g_gpio_spy.micros_calls++;
    g_gpio_spy.last_micros += 100;
    return g_gpio_spy.last_micros;
}

void delay(unsigned int ms)
{
    (void)ms;
}
