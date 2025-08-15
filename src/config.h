#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define NUM_SESSIONS 5
// NAO USAR A PORTA 12 - DA ERRO NO PLATAFORMIO AO ENVIAR O PGM PRO ESP32

// Sensores de umidade: ADC1_CH1 a ADC1_CH5 → GPIO 37, 38, 39, 32, 33
const uint8_t sensorPins[NUM_SESSIONS] = {37, 38, 39, 32, 33};

// Relés de irrigação: GPIO14 a GPIO18
const uint8_t relayPins[NUM_SESSIONS]  = {14, 15, 16, 17, 18};

// NAO USAR A PORTA 12 - DA ERRO NO PLATAFORMIO AO ENVIAR O PGM PRO ESP32
// Relé da bomba (única)
const uint8_t pumpPin  = 14;
// Relé da luz artificial (única)
const uint8_t lightPin = 13;

// Threshold de umidade (%): 30%
const uint8_t HUMIDITY_THRESHOLD_PERCENT = 30;
// Converte % para valor ADC (0–4095) de secura:
// (100–30)% → 70% de “seco” → 0.7*4095 ≃ 2866
const uint16_t HUMIDITY_THRESHOLD_READING = (uint16_t)(((100 - HUMIDITY_THRESHOLD_PERCENT) * 4095UL) / 100UL);

// Ciclo de irrigação: 10 minutos (milissegundos)
const unsigned long IRRIGATION_DURATION = 10UL * 60UL * 1000UL;
// Cooldown entre ciclos na mesma sessão: 1 hora
const unsigned long IRRIGATION_COOLDOWN = 1UL * 60UL * 60UL * 1000UL;

// Horário de início da luz artificial e duração (horas locais)
const uint8_t LIGHT_START_HOUR     = 10;
const uint8_t LIGHT_DURATION_HOURS = 6;

// Período “dia” em que nenhum recurso pode funcionar à noite.
const uint8_t DAY_START_HOUR = 6;
const uint8_t DAY_END_HOUR   = 20;

// Sensor de fluxo de água (pulsos) → ADC1_CH6 / GPIO34
const uint8_t FLOW_SENSOR_PIN     = 34;
// Verificar fluxo a cada 30 segundos
typedef unsigned long ul;
const ul FLOW_CHECK_MS            = 10UL * 1000UL;
// Mínimo de pulsos em FLOW_CHECK_MS para considerar fluxo OK
const unsigned int MIN_FLOW_PULSES = 100;

// tempo de backlight do LCD em segundos
const unsigned int LCD_BACKLIGHT_ON_TIME = 6; // Tempo em milissegundos para manter o backlight ligado

// Wi-Fi / NTP (preencha com sua rede e fuso)
const char* WIFI_SSID     = "al_capone";
const char* WIFI_PASSWORD = "Brooklyn1899";
// Fuso horário de Brasília
const char* TZ_INFO       = "America/Sao_Paulo";
const char* TIMEZONE = "<-03>3"; // UTC-3 (horário de Brasília)


// LCD I2C configuration
const uint8_t LCD_I2C_ADDR = 0x27; // Endereço I2C do display LCD
const uint8_t LCD_COLUMNS = 16;    // Número de colunas do display
const uint8_t LCD_ROWS    = 2;     // Número de linhas do display

// configuração de ON/OFF dos relés
// vamos usar a conexão inversa, ou seja, LOW = ON e HIGH = OFF
// Isso é útil para evitar problemas com relés que ficam ligados por padrão quando não recebem sinal
// (alguns relés podem ser acionados com HIGH, outros com LOW)
// Verifique o tipo de relé que você está usando e ajuste conforme necessário.
// Aqui, LOW significa que o relé está ativo (ligado) e HIGH significa que o relé está inativo (desligado).
// Isso é comum em relés de estado sólido ou relés de contato normalmente aberto (NO).
// Se você estiver usando relés de contato normalmente fechado (NC), pode ser necessário inverter esses valores.
#define RELAY_TYPE_NC // Descomente esta linha se estiver usando relés normalmente fechados (NC)
// #define RELAY_TYPE_NO // Descomente esta linha se estiver usando relés normalmente abertos (NO)
// RELAY_ON e RELAY_OFF são definidos de acordo com o tipo de relé - portando escolha apenas um deles
#ifdef RELAY_TYPE_NC
const uint8_t RELAY_ON = LOW;
const uint8_t RELAY_OFF = HIGH;
#endif
#ifdef RELAY_TYPE_NO
const uint8_t RELAY_ON = HIGH;
const uint8_t RELAY_OFF = LOW;  
#endif

// usar sensor de fluxo de água?
// Se você não tiver um sensor de fluxo, comente a linha abaixo.
// Isso desativará o sensor de fluxo e evitará erros de compilação.
//#define FLOW_SENSOR_ACTIVE

#endif // CONFIG_H
