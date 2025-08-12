#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include <WebServer.h>
#include <Preferences.h>

// Preferences instance
Preferences prefs;

// Dynamic configuration variables (initial defaults)
int cfgHumidPerct = HUMIDITY_THRESHOLD_PERCENT;
int cfgIrrgtDrtion = IRRIGATION_DURATION / 60000;
int cfgIrrgCoolDwn = IRRIGATION_COOLDOWN / 3600000;
int cfgLightStartH = LIGHT_START_HOUR;
int cfgLightDurationH = LIGHT_DURATION_HOURS;

// Derived runtime values
int cfgHumidityThresholdReading;
unsigned long cfgIrrigationDuration;
unsigned long cfgIrrigationCooldown;

// Function prototypes
void loadConfig();
void handleConfigGet();
void handleConfigPost();
WebServer server(80);

// Load configuration and define handlers
void loadConfig() {
  prefs.begin("config", false);
  cfgHumidPerct = prefs.getUInt("humidityThresholdPercent", cfgHumidPerct);
  cfgIrrgtDrtion = prefs.getUInt("irrigationDurationMin", cfgIrrgtDrtion);
  cfgIrrgCoolDwn = prefs.getUInt("irrigationCooldownH", cfgIrrgCoolDwn);
  cfgLightStartH = prefs.getUInt("lightStartHourVar", cfgLightStartH);
  cfgLightDurationH = prefs.getUInt("lightDurationH", cfgLightDurationH);
  prefs.end();
  Serial.printf("Configuração carregada:\n");
  Serial.printf("  Humidity threshold percent: %d\n", cfgHumidPerct);
  Serial.printf("  Irrigation duration (min): %d\n", cfgIrrgtDrtion);
  Serial.printf("  Irrigation cooldown (h): %d\n", cfgIrrgCoolDwn);
  Serial.printf("  Light start hour: %d\n", cfgLightStartH);
  Serial.printf("  Light duration (h): %d\n", cfgLightDurationH);

  // Derived values
  cfgHumidityThresholdReading = (int)(((100 - cfgHumidPerct) * 4095UL) / 100UL);
  cfgIrrigationDuration = (unsigned long)cfgIrrgtDrtion * 60000UL;
  cfgIrrigationCooldown = (unsigned long)cfgIrrgCoolDwn * 3600000UL;
}

void handleConfigGet() {
  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Configuração</title></head><body>";
  html += "<h1>Configuração de Parâmetros</h1>";
  html += "<form method=\"POST\" action=\"/config\">";
  html += "<label>Soil-moisture threshold (%) <input type=\"number\" name=\"humidityThresholdPercent\" min=\"0\" max=\"100\" value=\"" + String(cfgHumidPerct) + "\"></label><br>";
  html += "<label>Irrigation duration (min) <input type=\"number\" name=\"irrigationDurationMin\" min=\"1\" max=\"120\" value=\"" + String(cfgIrrgtDrtion) + "\"></label><br>";
  html += "<label>Irrigation cooldown (h) <input type=\"number\" name=\"irrigationCooldownH\" min=\"0\" max=\"24\" value=\"" + String(cfgIrrgCoolDwn) + "\"></label><br>";
  html += "<label>Light start hour (h) <input type=\"number\" name=\"lightStartHourVar\" min=\"0\" max=\"23\" value=\"" + String(cfgLightStartH) + "\"></label><br>";
  html += "<label>Light duration (h) <input type=\"number\" name=\"lightDurationH\" min=\"0\" max=\"24\" value=\"" + String(cfgLightDurationH) + "\"></label><br>";
  html += "<input type=\"submit\" value=\"Salvar\">";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleConfigPost() {
  if (server.hasArg("humidityThresholdPercent")) {
    Serial.printf("Novo threshold: {%s}\n", server.arg("humidityThresholdPercent").c_str());
    cfgHumidPerct = server.arg("humidityThresholdPercent").toInt();
  }
  if (server.hasArg("irrigationDurationMin")) {
    Serial.printf("Nova duração irrigação: {%s}\n", server.arg("irrigationDurationMin").c_str());
    cfgIrrgtDrtion = server.arg("irrigationDurationMin").toInt();
  }
  if (server.hasArg("irrigationCooldownH")) {
    Serial.printf("Novo cooldown irrigação: {%s}\n", server.arg("irrigationCooldownH").c_str());
    cfgIrrgCoolDwn = server.arg("irrigationCooldownH").toInt();
  }
  if (server.hasArg("lightStartHourVar")) {
    Serial.printf("Nova hora início luz: {%s}\n", server.arg("lightStartHourVar").c_str());
    cfgLightStartH = server.arg("lightStartHourVar").toInt();
  }
  if (server.hasArg("lightDurationH")) {
    Serial.printf("Nova duração luz: {%s}\n", server.arg("lightDurationH").c_str());
    cfgLightDurationH = server.arg("lightDurationH").toInt();
  }
  prefs.begin("config", false);
  prefs.putUInt("cfgHumidPerct", cfgHumidPerct);
  prefs.putUInt("cfgIrrgtDrtion", cfgIrrgtDrtion);
  prefs.putUInt("cfgIrrgCoolDwn", cfgIrrgCoolDwn);
  prefs.putUInt("cfgLightStartH", cfgLightStartH);
  prefs.putUInt("lightDurationH", cfgLightDurationH);
  prefs.end();
  cfgHumidityThresholdReading = (int)(((100 - cfgHumidPerct) * 4095UL) / 100UL);
  cfgIrrigationDuration = (unsigned long)cfgIrrgtDrtion * 60000UL;
  cfgIrrigationCooldown = (unsigned long)cfgIrrgCoolDwn * 3600000UL;
  server.sendHeader("Location", "/config");
  server.send(303, "text/plain", "");
}

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
  return h >= cfgLightStartH && h < (cfgLightStartH + cfgLightDurationH);
}
// Erase NVS partition and reinitialize it
// This is useful for resetting the configuration
// or if you want to start fresh without any saved data.
// Use with caution, as it will delete all saved preferences.
#include <nvs_flash.h>
void erasePreferences() {
    Serial.println("Resetting NVS...");
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
}
void setup() {
  Serial.begin(115200);
  // configura sensor de fluxo de água (pulsos)
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), onFlowPulse, RISING);
  // Uncomment to reset NVS
  // erasePreferences(); 
  connectWiFi();
  // load persisted configuration
  loadConfig();

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
  server.on("/config", HTTP_GET, handleConfigGet);
  server.on("/config", HTTP_POST, handleConfigPost);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Página não encontrada");
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
  /* teste de impressão 
  Serial.printf("Hora atual: %02d:%02d:%02d\n", hour, tm_info.tm_min, tm_info.tm_sec);
  Serial.printf("Bomba: %s, Luz: %s\n", pumpState ? "ON" : "OFF", lightState ? "ON" : "OFF");
  for (int i = 0; i < NUM_SESSIONS; i++) {
    Serial.printf("Sessão %d: %s\n", i, sessionActive[i] ? "ON" : "OFF");
  }
  */
  /*fim teste de impressão*/
  delay(1000);
}