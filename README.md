# Sistema IoT de monitoreo ambiental

## Objetivo
Desarrollar y simular un sistema IoT con ESP32 que mida variables ambientales, muestre las lecturas por el monitor serie, transmita datos a ThingSpeak mediante WiFi y active automáticamente un LED según las condiciones del ambiente.

## Componentes
- ESP32 DevKit V1.
- DHT22: temperatura y humedad (GPIO 15).
- Fotoresistor LDR: luminosidad relativa (GPIO 34).
- LED rojo con resistencia de 220 ohmios (GPIO 25).
- Wokwi para la simulación y ThingSpeak para visualizar los datos.

## Funcionamiento
El ESP32 toma lecturas cada 2 segundos. La lectura analógica del LDR se transforma a un porcentaje relativo entre 0 y 100; no representa lux calibrados. Los datos se envían a ThingSpeak cada 30 segundos por WiFi (`Wokwi-GUEST`, contraseña vacía). Se utiliza `millis()` para programar las tareas.

## Condiciones del actuador
El LED se enciende si la temperatura supera los 30 °C **o** si la luminosidad relativa es inferior al 30 %. Si ninguna condición se cumple, queda apagado.

## Configuración de ThingSpeak
1. Crear una cuenta y un canal llamado **Monitoreo ambiental**.
2. Habilitar Field 1: Temperatura (°C), Field 2: Humedad (%) y Field 3: Luminosidad relativa (%).
3. Copiar la **Write API Key** desde la pestaña **API Keys**.
4. Crear en Wokwi un archivo privado `secrets.h` con:
   ```cpp
   #define THINGSPEAK_API_KEY "TU_WRITE_API_KEY"
   ```
5. No subir `secrets.h` a GitHub: está excluido por `.gitignore`.

## Simulación
1. Abrir el proyecto en Wokwi y comprobar que el circuito coincida con `diagram.json`.
2. Agregar las bibliotecas de `libraries.txt`.
3. Crear `secrets.h` con la Write API Key.
4. Ejecutar la simulación y abrir el monitor serie.
5. Verificar que los tres campos se actualicen en ThingSpeak.

## Archivos
- `sketch.ino`: programa del ESP32.
- `diagram.json`: circuito de Wokwi.
- `libraries.txt`: dependencias.
- `.gitignore`: excluye credenciales.
- `secrets.h.example`: plantilla sin clave real.
- `README.md`: documentación.

## Consideraciones
- ThingSpeak se actualiza cada 30 segundos.
- El porcentaje del LDR es orientativo y depende del circuito.
- Si el DHT22 entrega una lectura inválida, no se envía información hasta contar con temperatura y humedad válidas.
