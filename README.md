# Sistema IoT de monitoreo ambiental

## Objetivo
Desarrollar y simular un sistema IoT con ESP32 que mida variables ambientales, transmita las mediciones a ThingSpeak mediante WiFi y active automáticamente un LED según las condiciones del ambiente.

## Componentes y conexiones
| Componente | Pin del ESP32 | Función |
|---|---:|---|
| DHT22 - DATA | GPIO 15 | Temperatura y humedad |
| LDR - AO | GPIO 34 | Luminosidad relativa |
| LED - ánodo | GPIO 25 | Actuador |
| LED - cátodo | GND mediante resistencia de 220 Ω | Protección del LED |
| DHT22 y LDR - VCC | 3V3 | Alimentación |
| DHT22 y LDR - GND | GND | Referencia común |

El archivo `diagram.json` contiene estas conexiones para Wokwi.

## Funcionamiento
El ESP32 mide temperatura y humedad con el DHT22 y lee el valor analógico del módulo fotoresistor LDR. Las mediciones se realizan cada 2 segundos. La luminosidad se representa como un porcentaje relativo de 0 a 100%; no es una medición calibrada en lux.

**Importante sobre el LDR:** en el módulo de fotoresistor de Wokwi, el valor analógico aumenta cuando hay menos luz. El programa invierte esa escala para que un porcentaje alto represente más luz.

## Condiciones de activación del LED
El LED se enciende cuando se cumple al menos una de estas condiciones:
- Temperatura superior a 30 °C.
- Luminosidad relativa inferior al 30 %.

La condición lógica es OR (O). Si ninguna condición se cumple, el LED permanece apagado.

## Conectividad y ThingSpeak
El ESP32 se conecta a la red simulada `Wokwi-GUEST` (contraseña vacía) y envía los datos mediante HTTP a ThingSpeak cada 30 segundos.

Configurar un canal llamado **Monitoreo ambiental** con estos campos:
- Field 1: Temperatura (°C).
- Field 2: Humedad (%).
- Field 3: Luminosidad relativa (%).

Copiar la **Write API Key** desde la pestaña **API Keys** del canal. ThingSpeak permite una actualización cada 15 segundos en el servicio gratuito; este proyecto envía cada 30 segundos.

### Proteger la clave
En Wokwi, crear un archivo llamado `secrets.h` con este contenido:

```cpp
#define THINGSPEAK_API_KEY "TU_WRITE_API_KEY"
```

No subir ese archivo a GitHub. El archivo `.gitignore` ya excluye `secrets.h`. El archivo `secrets.h.example` es solamente una plantilla sin credenciales reales.

## Bibliotecas
Agregar estas bibliotecas en Wokwi (Library Manager) o en `libraries.txt`:
- DHT sensor library (Adafruit).
- Adafruit Unified Sensor.

Las bibliotecas `WiFi.h` y `HTTPClient.h` forman parte del entorno ESP32.

## Cómo ejecutar en Wokwi
1. Crear un proyecto nuevo de tipo **ESP32**.
2. Copiar `sketch.ino` al archivo principal del proyecto.
3. Copiar `diagram.json` al archivo de diagrama.
4. Agregar las dos bibliotecas indicadas.
5. Crear el archivo privado `secrets.h` con la Write API Key del canal.
6. Iniciar la simulación y abrir el monitor serie a 115200 baudios.
7. Esperar el envío de los datos y comprobar que ThingSpeak actualice los campos 1, 2 y 3.
8. Usar los controles de los sensores en Wokwi para variar la temperatura y la iluminación y comprobar el LED.

## Visualización
En ThingSpeak, abrir la pestaña **Private View** o **Public View** y configurar al menos dos gráficos:
- Gráfico 1: Field 1 - Temperatura.
- Gráfico 2: Field 2 - Humedad.
También se puede agregar un tercer gráfico para Field 3 - Luminosidad relativa.

## Archivos del repositorio
- `sketch.ino`: código fuente Arduino/C++.
- `diagram.json`: conexiones del circuito Wokwi.
- `libraries.txt`: bibliotecas externas.
- `.gitignore`: exclusión de credenciales.
- `secrets.h.example`: plantilla de configuración sin claves.
- `README.md`: documentación del proyecto.
