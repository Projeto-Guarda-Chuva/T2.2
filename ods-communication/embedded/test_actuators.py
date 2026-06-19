import requests
import time

# Configuracao dos IPs dos motores
# Substitua pelos IPs reais que seus ESPs estao recebendo
MOTOR_FIXO_IP = "192.168.1.100" 
MOTOR_MOVEL_IP = "192.168.1.101"

def send_command(ip, command):
    url = f"http://{ip}/command"
    try:
        response = requests.post(url, data=command, timeout=2)
        print(f"Comando {command} enviado para {ip}. Resposta: {response.status_code}")
    except Exception as e:
        print(f"Erro ao enviar comando para {ip}: {e}")

def get_status(ip):
    url = f"http://{ip}/status"
    try:
        response = requests.get(url, timeout=2)
        print(f"Status {ip}: {response.json()}")
    except Exception as e:
        print(f"Erro ao obter status de {ip}: {e}")

if __name__ == "__main__":
    print("Iniciando testes de integracao...")
    
    # Teste de comunicacao simples
    print("\n--- Testando Motor Fixo ---")
    get_status(MOTOR_FIXO_IP)
    send_command(MOTOR_FIXO_IP, "A")
    time.sleep(1)
    send_command(MOTOR_FIXO_IP, "STOP")
    
    print("\n--- Testando Motor Movel ---")
    get_status(MOTOR_MOVEL_IP)
    send_command(MOTOR_MOVEL_IP, "S")
    time.sleep(1)
    send_command(MOTOR_MOVEL_IP, "P")
    
    print("\nTestes finalizados.")
