#include "../include/motor_movel.h"

static motor_estado_t g_estado = MOTOR_PARADO;
static int g_velocidade = VELOCIDADE_PADRAO;

void motor_movel_setup(void)
{
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);

    ledcSetup(CH_ENA, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PIN_ENA, CH_ENA);

    motor_parar(); /* estado inicial seguro */
}

void motor_parar(void)
{
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    ledcWrite(CH_ENA, 0);
    g_estado = MOTOR_PARADO;
    Serial.println("Motor: PARADO");
}

void motor_subir(void)
{
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    ledcWrite(CH_ENA, g_velocidade);
    g_estado = MOTOR_SUBINDO;
    Serial.println("Motor: SUBINDO");
}

void motor_descer(void)
{
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    ledcWrite(CH_ENA, g_velocidade);
    g_estado = MOTOR_DESCENDO;
    Serial.println("Motor: DESCENDO");
}

bool motor_movel_handle_id(int id)
{
    switch (id) {
        case ID_MOTOR_PARAR:
            motor_parar();
            return true;

        case ID_MOTOR_SUBIR:
            motor_subir();
            return true;

        case ID_MOTOR_DESCER:
            motor_descer();
            return true;

        default:
            return false; /* ID desconhecido para esta família */
    }
}

motor_estado_t motor_movel_get_estado(void)
{
    return g_estado;
}

const char *motor_movel_get_estado_str(void)
{
    switch (g_estado) {
        case MOTOR_SUBINDO:  return "subindo";
        case MOTOR_DESCENDO: return "descendo";
        default:              return "parado";
    }
}