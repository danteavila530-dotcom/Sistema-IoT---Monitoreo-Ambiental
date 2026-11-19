#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "secrets.h"  // Este archivo se crea en Wokwi y NO se sube a GitHub.

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* THINGSPEAK_SERVER = "http://api.thingspeak.com/update";

// Pines del circuito (deben coincidir con diagram.json).
#define DHT_PIN 15
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define LED_PIN 25

// Umbrales para la automatizacion.
const float TEMPERATURA_LIMITE = 30.0;
const int LUMINOSIDAD_LIMITE = 30;

DHT dht(DHT_PIN, DHT_TYPE);

// Tareas temporizadas con millis().
const unsigned long INTERVALO_LECTURA = 2000;
const unsigned long INTERVALO_ENVIO = 30000;
const unsigned long INTERVALO_WIFI = 10000;

unsigned long ultimoMuestreo = 0;
unsigned long ultimoEnvio = 0;
unsigned long ultimoIntentoWiFi = 0;

float temperatura = NAN;
float humedad = NAN;
int luminosidad = 0;

void conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Intentando conectar con Wokwi-GUEST...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void leerSensores() {
  // El DHT22 entrega temperatura en grados Celsius y humedad en porcentaje.
  float nuevaTemperatura = dht.readTemperature();
  float nuevaHumedad = dht.readHumidity();

  if (!isnan(nuevaTemperatura)) temperatura = nuevaTemperatura;
  if (!isnan(nuevaHumedad)) humedad = nuevaHumedad;

  // En el modulo LDR de Wokwi, el valor ADC aumenta cuando hay menos luz.
  // Se invierte la escala para que 0% represente oscuridad y 100% mucha luz.
  int lecturaLDR = analogRead(LDR_PIN);
  luminosidad = constrain(map(lecturaLDR, 4095, 0, 0, 100), 0, 100);

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" C | Humedad: ");
  Serial.print(humedad);
  Serial.print(" % | Luminosidad relativa: ");
  Serial.print(luminosidad);
  Serial.println(" %");
}

void controlarLED() {
  // El LED se enciende si hace demasiado calor O si hay poca luminosidad.
  bool temperaturaAlta = !isnan(temperatura) &&
                         temperatura > TEMPERATURA_LIMITE;
  bool pocaLuz = luminosidad < LUMINOSIDAD_LIMITE;

  digitalWrite(LED_PIN, (temperaturaAlta || pocaLuz) ? HIGH : LOW);
}

void enviarThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se enviaron datos: WiFi desconectado.");
    return;
  }

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("No se enviaron datos: lectura DHT22 invalida.");
    return;
  }

  HTTPClient http;
  String url = String(THINGSPEAK_SERVER) +
               "?api_key=" + THINGSPEAK_API_KEY +
               "&field1=" + String(temperatura, 1) +
               "&field2=" + String(humedad, 1) +
               "&field3=" + String(luminosidad);

  http.begin(url);
  int codigoHTTP = http.GET();

  if (codigoHTTP == HTTP_CODE_OK) {
    String respuesta = http.getString();
    Serial.print("Respuesta de ThingSpeak: ");
    Serial.println(respuesta);

    // ThingSpeak devuelve un ID numerico si guardo la medicion.
    if (respuesta.toInt() > 0) {
      Serial.println("Datos guardados correctamente.");
    } else {
      Serial.println("ThingSpeak no guardo los datos. Revisar API Key y canal.");
    }
  } else {
    Serial.print("Error HTTP al enviar: ");
    Serial.println(codigoHTTP);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  analogReadResolution(12);  // ADC del ESP32: valores de 0 a 4095.
  dht.begin();

  WiFi.mode(WIFI_STA);
  conectarWiFi();

  Serial.println("Sistema IoT de monitoreo ambiental iniciado.");
}

void loop() {
  unsigned long ahora = millis();

  // Reintenta conectarse a WiFi sin bloquear el ciclo principal.
  if (WiFi.status() != WL_CONNECTED &&
      ahora - ultimoIntentoWiFi >= INTERVALO_WIFI) {
    ultimoIntentoWiFi = ahora;
    conectarWiFi();
  }

  // Lectura de sensores cada 2 segundos.
  if (ahora - ultimoMuestreo >= INTERVALO_LECTURA) {
    ultimoMuestreo = ahora;
    leerSensores();
    controlarLED();
  }

  // Envio de las tres variables a ThingSpeak cada 30 segundos.
  if (ahora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = ahora;
    enviarThingSpeak();
  }
}
