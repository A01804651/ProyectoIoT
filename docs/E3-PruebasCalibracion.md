# KORA — Pruebas de calibración

Implementación de Internet de las Cosas · TC1004B.501 · Equipo 10 · Yamel Parra Rodriguez (A01798985) y Didier Alejandro Luna Contreras (A01804651)

## Resumen

El MAX30102 y el MPU6050 ya están calibrados; falta el DS18B20 y las pruebas de caídas. Todas las pruebas se hicieron con el ESP32 DevKit V1 y los sensores en protoboard.

| Sensor | Variable | Estado | Corrección aplicada |
| --- | --- | --- | --- |
| MAX30102 | Frecuencia cardiaca | Calibrado | Ninguna (diferencia de unos ±3 lpm) |
| MAX30102 | SpO2 | Sin validar (falta oxímetro) | Ninguna |
| MPU6050 | Aceleración y giro | Calibrado | Desfases por eje |
| MPU6050 | Detección de caídas | Pruebas pendientes | Umbrales iniciales |
| DS18B20 | Temperatura | Pendiente | Por calcular |

## MAX30102 (frecuencia cardiaca y SpO2)

La frecuencia cardiaca medida en la muñeca coincide con el pulso contado a mano con una diferencia de unos ±3 lpm, así que no necesita corrección.

**Configuración final**

| Parámetro | Valor |
| --- | --- |
| Posición | Muñeca |
| Brillo del LED | 35 (de 0 a 255) |
| Muestreo | 25 muestras/s |
| Señal IR en reposo | 123,000 a 127,000 (rango buscado: 100,000 a 200,000) |
| Método de FC | Detección de picos propia: promedio de 8 intervalos entre latidos |
| Método de SpO2 | Algoritmo de la librería SparkFun, promedio de 10 lecturas |

**Procedimiento.** Con el sensor en la muñeca y el antebrazo apoyado, se esperó a tener 8 latidos detectados. Luego se contó el pulso a mano durante 30 segundos (×2) y se comparó con el valor del sensor en ese momento. Se repitió varias veces.

**Resultados**

| Método | Resultado |
| --- | --- |
| Pulso contado a mano | 76 a 80 lpm (38 a 40 latidos en 30 s) |
| FC por detección de picos | 74 a 78 lpm |
| FC del algoritmo de la librería | Entre 42 y 166 lpm, sin estabilizarse |
| SpO2 | 93 a 96 % en reposo |

El algoritmo de la librería se descartó para la FC, porque también marcó de más en el dedo (92 a 104 lpm contra 80 contados). La SpO2 queda en un rango normal para una persona sana, pero no se pudo validar sin un oxímetro de referencia.

**Limitación principal.** El movimiento arruina la lectura: al mover la mano, la señal IR cambió entre 55,000 y 131,000, se dejaron de detectar latidos y la SpO2 bajó a 88 %. Por eso KORA descartará la FC y la SpO2 cuando el MPU6050 detecte movimiento.

## MPU6050 (detección de caídas)

Con los desfases corregidos, el sensor marca 1.00 g en reposo, como debe. Las pruebas de caídas siguen pendientes.

**Desfases.** Se promediaron 500 lecturas (10 s) con el sensor plano y quieto. El eje Z marcaba 1.287 g en lugar de 1.000 g, un desfase de fábrica común en estos módulos.

| Eje | Acelerómetro (g) | Giroscopio (°/s) |
| --- | --- | --- |
| X | 0.047 | 0.29 |
| Y | 0.050 | 2.62 |
| Z | 0.287 | 0.04 |

**Verificación.** Después de restar los desfases, el sensor plano marcó entre 0.99 y 1.01 g, con el giroscopio en 0.0 a 0.1 °/s. Un golpe registró 6.45 g sin saturarse, lo que confirma el rango de ±8 g.

**Umbrales iniciales.** Una caída se confirma cuando ocurren las tres fases en orden:

1. Caída libre: aceleración total menor a 0.5 g durante al menos 60 ms.
2. Golpe: mayor a 2.5 g en menos de 1 s después.
3. Inmovilidad: entre 0.7 y 1.3 g, con giro menor a 30 °/s, durante 2 s.

**Pruebas pendientes**

| Prueba | Resultado esperado | Resultado |
| --- | --- | --- |
| Caminar con el sensor | Sin caída | |
| Sentarse de golpe, aplaudir, sacudir la mano | Sin caída | |
| Soltar sobre una almohada desde 50 cm y dejar quieto | Caída detectada | |
| Soltar sobre la almohada y levantar enseguida | Sin caída (se movió) | |

## DS18B20 (temperatura)

Pendiente. En las pruebas iniciales el sensor fue estable: marcó 26.00 °C de ambiente, variando solo 0.06 °C (su resolución). En la muñeca marcará menos que la temperatura corporal, así que necesita una corrección.

**Procedimiento**

1. Colocar la sonda en la muñeca y esperar 10 minutos a que se estabilice.
2. Medir al mismo tiempo con un termómetro digital (axila o bajo la lengua).
3. Calcular la diferencia: corrección = termómetro − sensor.
4. Repetir 3 a 5 veces y promediar. En el código, temperatura = lectura + corrección.

| # | Sensor (°C) | Termómetro (°C) | Diferencia (°C) |
| --- | --- | --- | --- |
| 1 | | | |
| 2 | | | |
| 3 | | | |
