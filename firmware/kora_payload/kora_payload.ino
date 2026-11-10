/*
  KORA - Semana 5: construcción del payload JSON
  Equipo 10 - TC1004B.501

  Arma los dos mensajes JSON de KORA (medición y caída) con ArduinoJson
  y los imprime por serial. El formato sigue docs/E3-FormatoDatos-JSON-MQTT.md:
  los campos se llaman igual que las columnas de la base de datos.

  Por ahora usa valores de prueba; en la integración se reemplazan por
  los promedios reales del MAX30102 y el DS18B20, y por la detección de
  caídas del MPU6050. En la Semana 6 estos mismos payloads se publican
  por MQTT en kora/<mac>/medicion y kora/<mac>/caida.

  Librerías: ArduinoJson (Benoit Blanchon, v7)
  Placa: ESP32 Dev Module
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

// ---- Configuración ----
const char* ssid     = "UID";         // Red de 2.4 GHz
const char* password = "contraseña";  // No subir la contraseña real al repo

const long ZONA_HORARIA_S = -6 * 3600;           // Centro de México, sin horario de verano
const unsigned long INTERVALO_MEDICION_MS = 30000; // Resumen cada 30 s

String macAddress;
unsigned long ultimaMedicion = 0;

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
  macAddress = WiFi.macAddress();
  Serial.print("MAC: ");
  Serial.println(macAddress);
}

// Toma la hora real por NTP para llenar fecha_hora
void sincronizarHora() {
  configTime(ZONA_HORARIA_S, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Sincronizando hora");
  struct tm t;
  int intentos = 0;
  while (!getLocalTime(&t, 500) && intentos < 20) {
    Serial.print(".");
    intentos++;
  }
  Serial.println();
  Serial.println(intentos < 20 ? "Hora sincronizada" : "No se pudo obtener la hora por NTP");
}

// Devuelve la fecha y hora en formato "AAAA-MM-DDTHH:MM:SS"
String fechaHoraActual() {
  struct tm t;
  if (!getLocalTime(&t, 100)) return "";
  char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
  return String(buf);
}

// Un valor negativo indica que no hubo lectura válida en el intervalo -> null
void agregarValor(JsonDocument& doc, const char* campo, float valor) {
  if (valor < 0) doc[campo] = nullptr;
  else doc[campo] = round(valor * 10) / 10.0;  // un decimal
}

void publicarMedicion(float fc, float spo2, float temperatura) {
  JsonDocument doc;
  doc["mac_address"] = macAddress;
  doc["fecha_hora"]  = fechaHoraActual();
  agregarValor(doc, "frecuencia_cardiaca", fc);
  agregarValor(doc, "spo2", spo2);
  agregarValor(doc, "temperatura", temperatura);

  char payload[256];
  serializeJson(doc, payload);
  Serial.print("[medicion] ");
  Serial.println(payload);
}

void publicarCaida(float picoG) {
  JsonDocument doc;
  doc["mac_address"] = macAddress;
  doc["fecha_hora"]  = fechaHoraActual();
  doc["pico_g"]      = round(picoG * 100) / 100.0;  // dos decimales

  char payload[128];
  serializeJson(doc, payload);
  Serial.print("[caida] ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("KORA - Prueba de payload JSON");
  conectarWiFi();
  sincronizarHora();
  ultimaMedicion = millis() - INTERVALO_MEDICION_MS;  // primera medición inmediata
}

void loop() {
  if (millis() - ultimaMedicion >= INTERVALO_MEDICION_MS) {
    ultimaMedicion = millis();

    // Valores de prueba realistas con una pequeña variación
    float fc          = 72.0 + random(0, 80) / 10.0;   // 72.0 - 79.9 lpm
    float spo2        = 95.5 + random(0, 30) / 10.0;   // 95.5 - 98.4 %
    float temperatura = 36.3 + random(0, 5) / 10.0;    // 36.3 - 36.7 °C

    publicarMedicion(fc, spo2, temperatura);
  }

  // Prueba de caída: escribe "c" en el monitor serial para simular una
  // Prueba sin lectura: escribe "n" para mandar una medición con null
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') publicarCaida(3.84);
    if (c == 'n') publicarMedicion(-1, -1, 36.5);
  }
}
