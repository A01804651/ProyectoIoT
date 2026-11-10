/*
  KORA - Semana 5: construcción del payload JSON
  Equipo 10 - TC1004B.501

  Arma el payload JSON del reto con ArduinoJson y lo imprime por serial.
  Por ahora usa valores de prueba; en la integración se reemplazan por
  las lecturas reales del MAX30102, DS18B20 y MPU6050.

  Librerías: ArduinoJson (Benoit Blanchon, v7)
  Placa: ESP32 Dev Module
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

// ---- Configuración ----
const char* ssid     = "UID";         // Red de 2.4 GHz
const char* password = "contraseña";  // No subir la contraseña real al repo

#define DEVICE_ID "kora01"
const unsigned long INTERVALO_MS = 5000;

// Rangos normales (los mismos que usarán las alarmas en la Semana 9)
const float SPO2_MIN = 95.0;
const int   HR_MIN   = 60;
const int   HR_MAX   = 100;
const float TEMP_MIN = 36.0;
const float TEMP_MAX = 37.5;

bool horaOk = false;
unsigned long ultimoEnvio = 0;

void conectarWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Pide la hora real a un servidor NTP para que "ts" sea un timestamp Unix (UTC)
void sincronizarHora() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Sincronizando hora");
  for (int i = 0; i < 20 && time(nullptr) < 1700000000; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  horaOk = time(nullptr) >= 1700000000;
  Serial.println(horaOk ? "Hora sincronizada" : "Sin NTP: ts usara segundos desde el encendido");
}

unsigned long obtenerTimestamp() {
  if (horaOk) return (unsigned long)time(nullptr);
  return millis() / 1000;
}

void publicarLectura(float spo2, int hr, float tempC, bool caida) {
  bool alarma = (spo2 < SPO2_MIN) ||
                (hr < HR_MIN || hr > HR_MAX) ||
                (tempC < TEMP_MIN || tempC > TEMP_MAX) ||
                caida;

  JsonDocument doc;
  doc["device_id"] = DEVICE_ID;
  doc["ts"]        = obtenerTimestamp();
  doc["spo2"]      = spo2;
  doc["hr"]        = hr;
  doc["temp_c"]    = tempC;
  doc["fall"]      = caida;
  doc["alarm"]     = alarma;

  char payload[256];
  serializeJson(doc, payload);
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("KORA - Prueba de payload JSON");
  conectarWiFi();
  sincronizarHora();
}

void loop() {
  if (millis() - ultimoEnvio >= INTERVALO_MS) {
    ultimoEnvio = millis();

    // Valores de prueba realistas con una pequeña variación
    float spo2  = 96.5 + random(0, 20) / 10.0;   // 96.5 - 98.4 %
    int   hr    = 68 + random(0, 10);            // 68 - 77 lpm
    float tempC = 36.4 + random(0, 5) / 10.0;    // 36.4 - 36.8 °C
    bool  caida = false;

    publicarLectura(spo2, hr, tempC, caida);
  }
}
