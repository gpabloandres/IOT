#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11  // Tipo de sensor DHT
#define DHTPin 4       // Pin donde se conecta el sensor DHT

DHT dht(DHTPin, DHTTYPE);  // Inicializar el sensor DHT

// Credenciales para la conexión WiFi
const char* ssid = "ASP";
const char* password = "ASP110110";
const char* mqtt_server = "192.168.1.12";  // Dirección del broker MQTT

// Credenciales del broker MQTT (dejar en NULL si no se requieren)
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Inicialización del cliente WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;

void setup() {
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// Función para conectar el ESP8266 a la red WiFi
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

  Serial.println();
  Serial.print("WiFi conectado - Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función de callback para manejar mensajes MQTT entrantes
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Función para reconectar al servidor MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("conectado");
      client.publish("event", "hello world");
      client.subscribe("event");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  // Publicar datos cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humedad = dht.readHumidity();
    float temperaturaC = dht.readTemperature();  // Temperatura en Celsius

    if (isnan(humedad) || isnan(temperaturaC)) {
      Serial.println("Error al leer el sensor DHT!");
      return;
    }

    // Publicar valores de temperatura y humedad
    client.publish("tempaire/sensordht11", String(temperaturaC).c_str());
    client.publish("humidity/sensordht11", String(humedad).c_str());
    
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(temperaturaC);
    Serial.println(" ºC");
  }
}
