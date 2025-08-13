#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define NUM_SESSIONS 5

// Sensores de umidade: ADC1_CH1 a ADC1_CH5 → GPIO 37, 38, 39, 32, 33
const uint8_t sensorPins[NUM_SESSIONS] = {37, 38, 39, 32, 33};

// Relés de irrigação: GPIO14 a GPIO18
const uint8_t relayPins[NUM_SESSIONS]  = {14, 15, 16, 17, 18};

// Relé da bomba (única)
const uint8_t pumpPin  = 12;
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
const ul FLOW_CHECK_MS            = 30UL * 1000UL;
// Mínimo de pulsos em 30s para considerar fluxo OK
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

#endif // CONFIG_H
