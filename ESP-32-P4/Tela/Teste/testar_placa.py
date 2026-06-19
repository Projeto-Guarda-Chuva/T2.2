import requests
import sys

ESP32_IP = "192.168.76.43"
URL = f"http://{ESP32_IP}/cmd"

COMANDOS = {
    10: "Cor customizada",
    11: "Vermelho",
    12: "Verde",
    13: "Azul",
    14: "Preto",
    15: "Branco",
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
    print("\n=== Teste ESP32-P4 ===")
    print(f"IP: {ESP32_IP}")
    print("----------------------")
    for id_, nome in COMANDOS.items():
        print(f"  {id_} — {nome}")
    print("   0 — Sair")
    print("----------------------")

def main():
    global ESP32_IP, URL
    if len(sys.argv) > 1:
        ESP32_IP = sys.argv[1]
        URL = f"http://{ESP32_IP}/cmd"

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

        if escolha == 10:
            try:
                r = int(input("  R (0-255): "))
                g = int(input("  G (0-255): "))
                b = int(input("  B (0-255): "))
                enviar({"id": 10, "r": max(0,min(255,r)),
                                  "g": max(0,min(255,g)),
                                  "b": max(0,min(255,b))})
            except ValueError:
                print("  Valor invalido.")
        else:
            enviar({"id": escolha})

if __name__ == "__main__":
    main()
