#include "json_utils.h"

bool json_extrair_campo_int(const String &body, const char *campo, int &out_valor)
{
    String chave = String("\"") + campo + "\"";
    int pos = body.indexOf(chave);
    if (pos < 0) return false;

    int dois_pontos = body.indexOf(':', pos);
    if (dois_pontos < 0) return false;

    int i = dois_pontos + 1;
    while (i < (int)body.length() && isspace(body[i])) i++;

    bool negativo = false;
    if (i < (int)body.length() && body[i] == '-') {
        negativo = true;
        i++;
    }

    String numero = "";
    while (i < (int)body.length() && isDigit(body[i])) {
        numero += body[i];
        i++;
    }

    if (numero.length() == 0) return false;

    out_valor = numero.toInt() * (negativo ? -1 : 1);
    return true;
}
