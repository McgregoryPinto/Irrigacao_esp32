#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"

bool sessionActive[NUM_SESSIONS]   = {false};
unsigned long sessionStart[NUM_SESSIONS]  = {0};
unsigned long nextAllowed[NUM_SESSIONS]   = {0};

bool pumpState  = false;
bool lightState = false;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("OK");
  configTzTime(TZ_INFO, "pool.ntp.org", "time.nist.gov");
}

bool isDaytime(int h) {
  return h >= DAY_START_HOUR && h < DAY_END_HOUR;
}
bool isLightPeriod(int h) {
  return h >= LIGHT_START_HOUR && h < (LIGHT_START_HOUR + LIGHT_DURATION_HOURS);
}

void setup() {
  Serial.begin(115200);
  connectWiFi();

  for (int i = 0; i < NUM_SESSIONS; i++) {
    pinMode(sensorPins[i], INPUT);
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
  pinMode(pumpPin, OUTPUT);  digitalWrite(pumpPin, LOW);
  pinMode(lightPin, OUTPUT); digitalWrite(lightPin, LOW);
}

void loop() {
  struct tm tm_info;
  if (!getLocalTime(&tm_info)) {
    Serial.println("Erro NTP");
  }
  int hour    = tm_info.tm_hour;
  unsigned long now = millis();

  // 1) SESSÕES DE IRRIGAÇÃO
  bool anyOn = false;
  for (int i = 0; i < NUM_SESSIONS; i++) {

    if (sessionActive[i]) {
      // terminar ciclo após 10min ou se anoitecer
      if ((now - sessionStart[i] >= IRRIGATION_DURATION) || !isDaytime(hour)) {
        digitalWrite(relayPins[i], LOW);
        sessionActive[i] = false;
        nextAllowed[i]   = now + IRRIGATION_COOLDOWN;
        Serial.printf("Sessão %d → OFF\n", i);
      } else {
        anyOn = true;
      }

    } else {
      // iniciar se seco, for dia e cooldown passou
      int v = analogRead(sensorPins[i]);
      if (isDaytime(hour) && now >= nextAllowed[i] && v > HUMIDITY_THRESHOLD_READING) {
        digitalWrite(relayPins[i], HIGH);
        sessionActive[i] = true;
        sessionStart[i]  = now;
        anyOn = true;
        Serial.printf("Sessão %d → ON (leitura=%d)\n", i, v);
      }
    }
  }

  // 2) BOMBA (única) — ON se qualquer relé de irrigação ON
  if (anyOn && !pumpState) {
    digitalWrite(pumpPin, HIGH);
    pumpState = true;
    Serial.println("Bomba ON");
  } else if (!anyOn && pumpState) {
    digitalWrite(pumpPin, LOW);
    pumpState = false;
    Serial.println("Bomba OFF");
  }

  // 3) LUZ ARTIFICIAL (única)
  bool shouldLight = isDaytime(hour) && isLightPeriod(hour);
  if (shouldLight && !lightState) {
    digitalWrite(lightPin, HIGH);
    lightState = true;
    Serial.println("Luz ON");
  } else if (!shouldLight && lightState) {
    digitalWrite(lightPin, LOW);
    lightState = false;
    Serial.println("Luz OFF");
  }

  delay(1000);
}