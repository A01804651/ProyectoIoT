# KORA — Formato de datos (JSON y MQTT)

Implementación de Internet de las Cosas · TC1004B.501 · Equipo 10 · Yamel Parra Rodriguez (A01798985) y Didier Alejandro Luna Contreras (A01804651)

El ESP32 publica por MQTT dos tipos de mensaje en JSON: un resumen de mediciones cada 30 segundos y un aviso inmediato cuando detecta una caída.

## Flujo de los datos

1. **ESP32:** lee el MAX30102 a 25 muestras/s, el MPU6050 a 50 lecturas/s y el DS18B20 una vez por segundo. Calcula la FC, la SpO2 y la temperatura corregida, y detecta las caídas.
2. **ESP32 → Mosquitto:** cada 30 s publica los promedios del intervalo; si hay una caída, la publica en ese momento.
3. **Node-RED:** recibe los mensajes, busca el dispositivo por su MAC, revisa los rangos y genera las alertas.
4. **MariaDB:** Node-RED inserta las mediciones en Medicion y las alertas en Alerta.
5. **Grafana y notificaciones:** el dashboard consulta MariaDB, y Node-RED avisa a los usuarios que supervisan al paciente.

## Temas MQTT

| Tema | Contenido | Frecuencia | QoS |
| --- | --- | --- | --- |
| `kora/<mac>/medicion` | Promedios de FC, SpO2 y temperatura | Cada 30 s | 0 |
| `kora/<mac>/caida` | Aviso de caída con el pico de aceleración | Al detectarse | 1 |

Las caídas usan QoS 1 para que el broker confirme que llegaron; perder una medición no es grave porque llega otra a los 30 s.

## Mensajes JSON

**Medición** (`kora/<mac>/medicion`):

```json
{
  "mac_address": "20:50:0D:29:85:F4",
  "fecha_hora": "2026-11-10T18:30:00",
  "frecuencia_cardiaca": 78.4,
  "spo2": 96.1,
  "temperatura": 36.5
}
```

**Caída** (`kora/<mac>/caida`):

```json
{
  "mac_address": "20:50:0D:29:85:F4",
  "fecha_hora": "2026-11-10T18:31:12",
  "pico_g": 3.84
}
```

## Correspondencia con la base de datos

Los campos se llaman igual que las columnas, para que Node-RED los inserte directo.

| Campo del JSON | Tabla.columna | Origen en el ESP32 |
| --- | --- | --- |
| mac_address | Dispositivo.mac_address | `WiFi.macAddress()`; Node-RED obtiene el id_dispositivo con ella |
| fecha_hora | Medicion.fecha_hora / Alerta.fecha_hora | Hora real por NTP |
| frecuencia_cardiaca | Medicion.frecuencia_cardiaca | Promedio de la FC por detección de picos |
| spo2 | Medicion.spo2 | Promedio de la SpO2 del intervalo |
| temperatura | Medicion.temperatura | Promedio del DS18B20 + su corrección |
| Mensaje de caída | Alerta (tipo_alerta = caida, nivel = alto) | Detección de caídas del MPU6050 |

## Reglas

- **Sin lectura válida se manda `null`:** si el sensor no toca la piel o no hubo latidos válidos en el intervalo. Las columnas de FC, SpO2 y temperatura aceptan nulos.
- **Movimiento:** si el MPU6050 detecta movimiento, las lecturas de FC y SpO2 de ese momento se descartan del promedio.
- **Hora:** el ESP32 toma la hora por NTP al conectarse a WiFi, así que la medición queda con la hora en que se tomó.
- **Caídas en el ESP32, lo demás en Node-RED:** la caída necesita 50 lecturas por segundo y no puede enviarse por MQTT, por eso la detecta el ESP32. Las alertas de FC, SpO2 y temperatura las decide Node-RED.

**Rangos propuestos para las alertas en Node-RED** (por validar con el equipo)

| tipo_alerta | Condición | nivel |
| --- | --- | --- |
| fc_baja | FC menor a 50 lpm | medio |
| fc_alta | FC mayor a 120 lpm en reposo | medio |
| spo2_baja | SpO2 menor a 92 % (menor a 88 %: alto) | medio / alto |
| temp_alta | Temperatura mayor a 37.5 °C (38 °C o más: alto) | medio / alto |
| temp_baja | Temperatura menor a 35 °C | medio |
| caida | Mensaje en `kora/<mac>/caida` | alto |

Pendiente de decidir: si se regresa `aceleracion_max` a la tabla Medicion para guardar también el movimiento del intervalo.
