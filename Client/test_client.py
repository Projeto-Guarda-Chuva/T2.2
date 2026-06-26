import requests
import sys

ESP32_IP = "192.168.76.43"
ENDPOINT = "/cmd"
URL = f"http://{ESP32_IP}{ENDPOINT}"

COMANDOS = {
    # Grupo 10 — Display / Cores
    10: "Cor customizada",
    11: "Vermelho",
    12: "Verde",
    13: "Azul",
    14: "Preto (Limpar)",
    15: "Branco",
    # Grupo 20 — Motor Fixo
    20: "Motor Fixo - Parar",
    21: "Motor Fixo - Abrir",
    22: "Motor Fixo - Fechar",
    # Grupo 30 — Motor Movel
    30: "Motor Movel - Parar",
    31: "Motor Movel - Subir",
    32: "Motor Movel - Descer",
    # Grupo 40 — Audio
    40: "Audio - Parar",
    41: "Audio - Reproduzir",
    42: "Audio - Definir Volume",
}


def enviar(payload):
    try:
        r = requests.post(URL, json=payload, timeout=5)
        print(f"  Resposta: {r.status_code} {r.text}")
    except requests.exceptions.ConnectionError:
        print(f"  Erro: nao foi possivel conectar em {URL}")
    except requests.exceptions.Timeout:
        print("  Erro: timeout")


def menu():
    print("\n=== Teste ESP32-P4 / Jetson - Protocolo Jellyfish V2 ===")
    print(f"URL: {URL}")
    print("----------------------")
    print("Grupo 10 - Display / Cores:")
    for id_ in (10, 11, 12, 13, 14, 15):
        print(f"  {id_} - {COMANDOS[id_]}")
    print("Grupo 20 - Motor Fixo:")
    for id_ in (20, 21, 22):
        print(f"  {id_} - {COMANDOS[id_]}")
    print("Grupo 30 - Motor Movel:")
    for id_ in (30, 31, 32):
        print(f"  {id_} - {COMANDOS[id_]}")
    print("Grupo 40 - Audio:")
    for id_ in (40, 41, 42):
        print(f"  {id_} - {COMANDOS[id_]}")
    print("----------------------")
    print("   0 - Sair")
    print("----------------------")


def pedir_inteiro(rotulo, minimo=None, maximo=None):
    valor = int(input(rotulo))
    if minimo is not None and maximo is not None:
        valor = max(minimo, min(maximo, valor))
    return valor


def main():
    global ESP32_IP, ENDPOINT, URL
    if len(sys.argv) > 1:
        ESP32_IP = sys.argv[1]
    if len(sys.argv) > 2:
        ENDPOINT = sys.argv[2]
        if not ENDPOINT.startswith("/"):
            ENDPOINT = "/" + ENDPOINT
    URL = f"http://{ESP32_IP}{ENDPOINT}"

    while True:
        menu()
        try:
            escolha = int(input("ID: ").strip())
        except ValueError:
            print("  Valor invalido.")
            continue

        if escolha == 0:
            print("Saindo.")
            break

        if escolha not in COMANDOS:
            print("  ID invalido.")
            continue

        # --- Grupo 10: ID 10 exige RGB ---
        if escolha == 10:
            try:
                r = pedir_inteiro("  R (0-255): ", 0, 255)
                g = pedir_inteiro("  G (0-255): ", 0, 255)
                b = pedir_inteiro("  B (0-255): ", 0, 255)
                enviar({"id": 10, "r": r, "g": g, "b": b})
            except ValueError:
                print("  Valor invalido.")

        # --- Grupo 40: ID 41 exige nome do arquivo ---
        elif escolha == 41:
            nome_arquivo = input("  Nome do arquivo (ex: alerta.mp3): ").strip()
            if not nome_arquivo:
                print("  Nome de arquivo invalido.")
                continue
            enviar({"id": 41, "file": nome_arquivo})

        # --- Grupo 40: ID 42 exige volume ---
        elif escolha == 42:
            try:
                volume = pedir_inteiro("  Volume (0-100): ", 0, 100)
                enviar({"id": 42, "volume": volume})
            except ValueError:
                print("  Valor invalido.")

        # --- Todos os IDs simples: 11-15, 20-22, 30-32, 40 ---
        else:
            enviar({"id": escolha})


if __name__ == "__main__":
    main()