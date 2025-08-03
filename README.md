# Irrigação Automática com ESP32

Este projeto implementa um sistema de monitoramento e irrigação automática de hortas ou plantações utilizando um microcontrolador ESP32. O sistema lê sensores de umidade do solo, aciona relés para irrigação, gerencia tempos de irrigação e cooldown, além de controlar bomba d’água e luz artificial.

## Funcionalidades

- Monitoramento de até 5 sessões de irrigação (sensores de umidade de solo).
- Acionamento de relés para válvulas de irrigação, bomba de água e luz artificial.
- Ciclo de irrigação de **10 minutos** com cooldown de **1 hora** entre ciclos na mesma sessão.
- Controle do período diurno (6h–20h) e de luz artificial (início às 10h com duração de 6h).
- Servidor web integrado para visualização do status via HTTP.
- Interface web para configuração interativa de parâmetros (umidade do solo, duração de irrigação, cooldown, horário de luz), com persistência na memória não-volátil do ESP32 via biblioteca Preferences.

## Requisitos de Hardware

- Placa ESP32 DOIT DevKit V1 (ESP32-D).
- 5 sensores de umidade de solo conectados aos pinos GPIO 37, 38, 39, 32 e 33.
- 5 módulos de relé para as sessões de irrigação (GPIO 14–18).
- Módulo de relé para a bomba de água (GPIO 12).
- Módulo de relé para a luz artificial (GPIO 13).
- (Opcional) Sensor de fluxo de água conectado ao GPIO 34.

## Configuração de Software

### Ferramentas Necessárias

- [PlatformIO](https://platformio.org/) (extensão para VSCode ou CLI).
- Arduino framework para ESP32.

### Passos de Instalação

- Instale o PlatformIO CLI (`pip install platformio`) ou use a extensão PlatformIO no VSCode.

1. Clone o repositório:
   ```bash
   git clone https://github.com/McgregoryPinto/Irrigacao_esp32.git
   cd Irrigacao_esp32
   ```
2. Edite `src/config.h` para definir:
   - `WIFI_SSID` e `WIFI_PASSWORD` da sua rede.
   - `TZ_INFO` (ex.: "America/Sao_Paulo").
   - Outros parâmetros de irrigação e luz conforme necessidade.
3. Compile para o ESP32 (placa `esp32doit-devkit-v1`):
   ```bash
   pio run -e esp32doit-devkit-v1
   ```
4. Faça upload para o ESP32:
   ```bash
   pio run -e esp32doit-devkit-v1 -t upload
   ```

## Uso

1. Conecte-se à rede Wi-Fi definida em `config.h`.
2. Abra o Monitor Serial em **115200** baud para ver o IP atribuído:
   ```
   WiFi...OK
   192.168.0.123
   ```
3. Acesse no navegador:
   ```
   http://<IP_do_ESP32>/
   ```
4. A página web informará o estado da luz, bomba e sessões de irrigação.

## Configuração via Web UI

Após conectar ao dispositivo à rede Wi-Fi e abrir o IP no navegador:

1. Acesse `http://<IP_do_ESP32>/config` para abrir a página de configuração.
2. Ajuste os parâmetros conforme necessidade:
   - **Soil-moisture threshold (%)**: percentual de umidade para disparo da irrigação.
   - **Irrigation duration (min)**: duração de cada ciclo de irrigação em minutos.
   - **Irrigation cooldown (h)**: tempo mínimo em horas entre ciclos na mesma sessão.
   - **Light start hour (h)**: hora do dia (0–23) para início da luz artificial.
   - **Light duration (h)**: duração da luz artificial em horas.
3. Clique em **Salvar** para persistir as configurações na memória não-volátil do ESP32.


## Estrutura do Projeto

- **platformio.ini**: configurações do build e do ambiente.
- **src/config.h**: constantes de hardware e parâmetros de controle.
- **src/main.cpp**: lógica principal de leitura de sensores, acionamento de relés e servidor web.

## Licença

Este projeto está licenciado sob a licença MIT. Sinta-se à vontade para modificar, usar e distribuir.
