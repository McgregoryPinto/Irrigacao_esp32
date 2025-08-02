#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include <WebServer.h>
WebServer server(80);

// --- Sensor de fluxo de água ---
volatile unsigned int flowPulseCount = 0;
unsigned long lastFlowCheck = 0;

void IRAM_ATTR onFlowPulse() {
  flowPulseCount++;
}



bool sessionActive[NUM_SESSIONS]   = {false};
unsigned long sessionStart[NUM_SESSIONS]  = {0};
unsigned long nextAllowed[NUM_SESSIONS]   = {0};

bool pumpState  = false;
bool lightState = false;

void connectWiFi() {
  char charIP[30]={};
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("OK");
  sprintf(charIP, "%s", WiFi.localIP().toString());
  Serial.println(charIP);
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
  // configura sensor de fluxo de água (pulsos)
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), onFlowPulse, RISING);

  connectWiFi();

  for (int i = 0; i < NUM_SESSIONS; i++) {
    pinMode(sensorPins[i], INPUT);
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
  pinMode(pumpPin, OUTPUT);  digitalWrite(pumpPin, LOW);
  pinMode(lightPin, OUTPUT); digitalWrite(lightPin, LOW);
  server.on("/", HTTP_GET, [](){
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Monitoramento Horta</title></head><body>";
    html += "<h1>Monitoramento da Horta</h1>";
    html += "<p><strong>Luz</strong>: Início às " + String(LIGHT_START_HOUR) + ":00 por " + String(LIGHT_DURATION_HOURS) + "h</p>";
    html += "<p><strong>Irrigação</strong>: Duração " + String(IRRIGATION_DURATION / 60000) + "min, intervalo " + String(IRRIGATION_COOLDOWN / 3600000) + "h</p>";
    html += "<p><strong>Bomba</strong>: " + String(pumpState ? "ON" : "OFF") + "</p>";
    for (int i = 0; i < NUM_SESSIONS; i++) {
      html += "<p><strong>Relé " + String(i) + "</strong>: " + String(sessionActive[i] ? "ON" : "OFF") + "</p>";
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  server.begin();
  Serial.println("=== Configuração Iniciada ===");
  Serial.printf("Sessões: %d\n", NUM_SESSIONS);
  Serial.print("Duração irrigação: "); Serial.print(IRRIGATION_DURATION / 60000); Serial.println(" min");
  Serial.print("Cooldown irrigação: "); Serial.print(IRRIGATION_COOLDOWN / 3600000); Serial.println(" h");
  Serial.print("Luz: inicia às "); Serial.print(LIGHT_START_HOUR); Serial.print(" h por "); Serial.print(LIGHT_DURATION_HOURS); Serial.println(" h");
  Serial.println("Servidor HTTP iniciado.");
  Serial.print("Acesse em: http://"); Serial.print(WiFi.localIP()); Serial.println("/");
  Serial.println("===========================");
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
  server.handleClient();

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