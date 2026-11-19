#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "secrets.h"

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* THINGSPEAK_SERVER = "http://api.thingspeak.com/update";

#define DHT_PIN 15
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define LED_PIN 25

const float TEMPERATURA_LIMITE = 30.0;
const int LUMINOSIDAD_LIMITE = 30;

DHT dht(DHT_PIN, DHT_TYPE);

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
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void leerSensores() {
  float nuevaTemperatura = dht.readTemperature();
  float nuevaHumedad = dht.readHumidity();

  if (!isnan(nuevaTemperatura)) temperatura = nuevaTemperatura;
  if (!isnan(nuevaHumedad)) humedad = nuevaHumedad;

  int lecturaLDR = analogRead(LDR_PIN);
  luminosidad = constrain(map(lecturaLDR, 0, 4095, 0, 100), 0, 100);

  Serial.print("Temperatura: "); Serial.print(temperatura);
  Serial.print(" C | Humedad: "); Serial.print(humedad);
  Serial.print(" % | Luminosidad relativa: "); Serial.print(luminosidad);
  Serial.println(" %");
}

void controlarLED() {
  // Se enciende si la temperatura supera 30 C O la luz es menor al 30 %.
  bool activarLED = (!isnan(temperatura) && temperatura > TEMPERATURA_LIMITE)
                    || luminosidad < LUMINOSIDAD_LIMITE;
  digitalWrite(LED_PIN, activarLED ? HIGH : LOW);
}

void enviarThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se envio: WiFi desconectado.");
    return;
  }
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("No se envio: lectura DHT22 no valida.");
    return;
  }

  HTTPClient http;
  String url = String(THINGSPEAK_SERVER) + "?api_key=" + THINGSPEAK_API_KEY
             + "&field1=" + String(temperatura, 1)
             + "&field2=" + String(humedad, 1)
             + "&field3=" + String(luminosidad);
  http.begin(url);
  int codigoHTTP = http.GET();

  if (codigoHTTP > 0) {
    Serial.print("ThingSpeak respondio: ");
    Serial.println(http.getString());
  } else {
    Serial.print("Error al enviar: ");
    Serial.println(http.errorToString(codigoHTTP));
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  analogReadResolution(12);
  dht.begin();
  WiFi.mode(WIFI_STA);
  conectarWiFi();
  Serial.println("Sistema IoT de monitoreo ambiental iniciado.");
}

void loop() {
  unsigned long ahora = millis();

  if (WiFi.status() != WL_CONNECTED &&
      ahora - ultimoIntentoWiFi >= INTERVALO_WIFI) {
    ultimoIntentoWiFi = ahora;
    conectarWiFi();
  }

  if (ahora - ultimoMuestreo >= INTERVALO_LECTURA) {
    ultimoMuestreo = ahora;
    leerSensores();
    controlarLED();
  }

  if (ahora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = ahora;
    enviarThingSpeak();
  }
}
