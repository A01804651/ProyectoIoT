# KORA — Esquema de datos (Semana 5) · BORRADOR

Equipo 10 · TC1004B.501
Yamel Parra Rodriguez A01798985 · Didier Alejandro Luna Contreras A01804651

## Criterios de diseño

El esquema toma la filosofía del modelo IPSO sin implementar LwM2M:

- **Nombres claros:** `temp_c` en lugar de `t`; `spo2` en lugar de `val1`.
- **Unidad explícita:** va en el nombre del campo o en esta tabla.
- **Origen identificado:** cada mensaje lleva `device_id` y `ts`.
- **Rango normal definido:** se usa para calcular `alarm` y servirá para la lógica de alarmas de la Semana 9.

Se parte del esquema de referencia del reto y se extiende con `fall`, porque KORA detecta caídas con el MPU6050.

## Tabla de variables

| Campo | Descripción | Sensor | Unidad | Tipo | Rango normal |
|---|---|---|---|---|---|
| `device_id` | Identificador del dispositivo | — | — | string | — |
| `ts` | Momento de la lectura (Unix, UTC) | NTP | s | entero | — |
| `spo2` | Saturación de oxígeno | MAX30102 | % | decimal | 95 – 100 |
| `hr` | Frecuencia cardiaca | MAX30102 | lpm | entero | 60 – 100 |
| `temp_c` | Temperatura corporal | DS18B20 | °C | decimal | 36.0 – 37.5 |
| `fall` | Caída detectada | MPU6050 | — | booleano | `false` |
| `alarm` | Algún valor fuera de rango o caída | Calculado | — | booleano | `false` |

## Ejemplo de payload

```json
{
  "device_id": "kora01",
  "ts": 1794326400,
  "spo2": 97.4,
  "hr": 72,
  "temp_c": 36.6,
  "fall": false,
  "alarm": false
}
```

El payload mide unos 100 bytes, así que cabe completo en un solo paquete WiFi (MTU de 1500 bytes) sin fragmentarse.

## Código del payload

El sketch está en [`firmware/kora_payload/kora_payload.ino`](../firmware/kora_payload/kora_payload.ino). Usa ArduinoJson para armar el documento y `serializeJson()` para imprimirlo.

A diferencia del ejemplo de clase, `ts` no se calcula con `millis()/1000`, que solo da los segundos desde el encendido. El ESP32 pide la hora real a un servidor NTP con `configTime()`, así que `ts` es un timestamp Unix válido para graficar series de tiempo. Si NTP falla, el sketch vuelve a `millis()/1000`.

## Evidencia del monitor serial

[Pegar aquí la captura del monitor serial con varios payloads impresos]
