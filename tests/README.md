# Resultado dos testes

[![Status do QA Central](https://github.com/Projeto-Guarda-Chuva/.github/actions/workflows/qa-t22.yml/badge.svg)](https://github.com/Projeto-Guarda-Chuva/.github/actions)

# Suíte de Testes Unitários - Camada T2.2

## Objetivo

Esta suíte foi elaborada para validar, de forma isolada, partes testáveis da camada `T2.2`
sem dependência de hardware físico, rede Wi-Fi real, servidor HTTP embarcado ou execução
integral do ambiente ESP-IDF.

O foco é verificar o comportamento das unidades de software cuja lógica pode ser exercitada
fora do dispositivo, preservando a implementação original da camada e concentrando os testes
em artefatos específicos de validação.

## Estrutura da suíte

Os testes estão organizados da seguinte forma:

- `tests/python/test_testar_placa.py`
  - valida o comportamento do script Python auxiliar utilizado para envio de comandos HTTP à placa
- `tests/c/test_protocol.c`
  - valida a interpretação de comandos JSON e o mapeamento de ações em `protocol.c`
- `tests/c/test_http_server.c`
  - valida a inicialização do servidor HTTP da tela e o encaminhamento de comandos recebidos
- `tests/c/test_lighting_manager.c`
  - valida a inicialização e aplicação de cor/intensidade no módulo de iluminação
- `tests/c/test_wifi_manager.c`
  - valida a inicialização do fluxo Wi-Fi em modo estação e a configuração das credenciais
- `tests/c/test_tela_main.c`
  - valida a orquestração principal da aplicação da tela, incluindo NVS, task de display, espera de Wi-Fi e inicialização do servidor HTTP
- `tests/c/test_led_main.c`
  - valida a orquestração principal da aplicação de iluminação e a sequência de cores aplicada
- `tests/c/stubs/`
  - contém stubs e implementações auxiliares usados para isolar dependências externas

## Estratégia de teste

Considerando a natureza embarcada da camada, a suíte adota isolamento por stubs para simular:

- componentes do ESP-IDF
- primitivas do FreeRTOS
- serviços HTTP embarcados
- stack de Wi-Fi e NVS do ESP-IDF
- driver de fita LED
- chamadas entre módulos da aplicação
- funções auxiliares de runtime
- dependências de parsing JSON

Essa abordagem permite validar a lógica interna das unidades selecionadas sem alterar o
código-fonte de produção e sem exigir infraestrutura física para execução dos testes.

## Execução

Para executar toda a suíte:

```bash
cd T2.2
bash tests/run_tests.sh
```

O script executa, em sequência:

1. os testes unitários em Python
2. a compilação e execução dos testes unitários em C no ambiente host

Durante a execução, os binários temporários são gerados em `tests/build/` apenas para a
sessão corrente e removidos automaticamente ao final do script.

## Escopo e limitações

Esta suíte não tem como objetivo substituir testes de integração ou testes em hardware.
Portanto, não estão cobertos por esta etapa:

- conectividade Wi-Fi real entre placas
- servidor HTTP real em execução no dispositivo
- interação com display físico
- execução completa sobre ESP32 com ESP-IDF em tempo de execução
- validação fim a fim entre múltiplas placas

Esses cenários devem ser tratados, quando necessário, em uma estratégia complementar de
testes de integração, testes em bancada e validação em dispositivo real.
