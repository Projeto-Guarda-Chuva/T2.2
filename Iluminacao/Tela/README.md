# ESP32-P4 — Controle de Display

## Todo novo terminal para ativar o ambiente

```bash
. ~/esp/esp-idf/export.sh

# apagar flash
idf.py erase-flash
```

## Compilar




```bash

# so na primeira vez ou ao trocar de chip
idf.py set-target esp32p4

# compilar
idf.py build
```




---

## Gravar e abrir monitor serial

```bash
idf.py flash monitor

idf.py erase-flash build flash monitor
```

Sair do monitor: `Ctrl + ]`

---

## Limpar build (quando algo der errado)

```bash
rm -rf build
idf.py set-target esp32p4
idf.py build
```

---

## Protocolo HTTP

Enviar `POST` para `http://<IP_DA_PLACA>/cmd` com JSON:

| id | Acao                       |
| -- | -------------------------- |
| 10 | Cor customizada`{r,g,b}` |
| 11 | Vermelho                   |
| 12 | Verde                      |
| 13 | Azul                       |
| 14 | Preto                      |
| 15 | Branco                     |

Exemplo:

```bash
curl -X POST http://192.168.76.43/cmd \
     -H "Content-Type: application/json" \
     -d '{"id": 13}'

curl -X POST http://192.168.76.43/cmd \
     -H "Content-Type: application/json" \
     -d '{"id": 10, "r": 128, "g": 0, "b": 255}'
```

---

## Wi-Fi

Edite `main/source/wifi_manager.c`:

```c
#define WIFI_SSID "NOME_DA_SUA_REDE"
#define WIFI_PASS "SENHA_DA_SUA_REDE"
```
