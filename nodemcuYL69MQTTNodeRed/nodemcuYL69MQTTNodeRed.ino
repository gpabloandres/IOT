#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración WiFi
const char* ssid = "ASP";
const char* password = "ASP110110";

// Configuración del servidor MQTT
const char* mqtt_server = "192.168.1.17";
const char* mqtt_topic = "humedsuelo/sensoryl69";

// Pines del sensor
const int sensorPin = A0;

// Valores de calibración
const int valorSeco = 442;  // Ajusta este valor según tu calibración
const int valorHumedo = 285; // Ajusta este valor según tu calibración

// Variables globales
WiFiClient espClient;
PubSubClient client(espClient);

// Función para conectarse a la red Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectarse al servidor MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leer el valor del sensor
  int sensorValue = analogRead(sensorPin);
  // Convertir el valor a porcentaje de humedad
  float humedad = map(sensorValue, valorSeco, valorHumedo, 0, 100);

  // Limitar el valor de humedad al rango 0-100
  humedad = constrain(humedad, 0, 100);

  // Imprimir el valor serialmente
  Serial.print("Humedad del suelo: ");
  Serial.print(humedad);
  Serial.println("%");

  char humiditynow [15];
  dtostrf(humedad, 7, 3, humiditynow); //// convert float to char
  client.publish(mqtt_topic, humiditynow); /// send char

  // Esperar 10 segundos antes de la próxima lectura
  delay(10000);
}
