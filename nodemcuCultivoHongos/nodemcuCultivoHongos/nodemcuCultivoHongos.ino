#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPin 4
#define PUMP_PIN 0  // Pin de la bomba de agua

DHT dht(DHTPin, DHTTYPE);

// Cambia las credenciales abajo para que tu ESP8266 se conecte a tu router
const char* ssid = "ASP";
const char* password = "ASP110110";

// Servidor MQTT
const char* mqtt_server = "192.168.1.17";

// Credenciales del broker MQTT (pon NULL si no se requieren)
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Inicializa el espClient. Cambia el nombre si tienes múltiples ESPs en tu sistema de automatización
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;

void setup() {
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(PUMP_PIN, OUTPUT);  // Configura el pin de la bomba de agua como salida
  digitalWrite(PUMP_PIN, LOW);  // Asegúrate de que la bomba esté apagada inicialmente
}

// Esta función conecta tu ESP8266 a tu router
void setup_wifi() {
  delay(10);
  // Comenzamos conectándonos a la red WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Función para reconectar al servidor MQTT
void reconnect() {
  // Bucle hasta que nos reconectemos
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Intentar conectar
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Una vez conectado, publicar un anuncio...
      client.publish("event", "hello world");
      // ... y resuscribirse
      client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Espera 5 segundos antes de volver a intentar
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NodeMCUClient", MQTT_username, MQTT_password);
  
  unsigned long currentMillis = millis();
  // Publica nueva temperatura y humedad cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    // Leer temperatura en Celsius (por defecto)
    float temperatureC = dht.readTemperature();
    // Leer temperatura en Fahrenheit (isFahrenheit = true)
    float temperatureF = dht.readTemperature(true);

    // Verificar si alguna lectura falló y salir temprano (para intentarlo de nuevo).
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Publicar valores de temperatura y humedad
    client.publish("tempaire/sensordht11", String(temperatureC).c_str());
    client.publish("humidity/sensordht11", String(humidity).c_str());
    // Descomentar para publicar temperatura en grados F
    // client.publish("iotfrontier/temperature", String(temperatureF).c_str());
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
    // Serial.print(temperatureF);
    // Serial.println(" ºF");

    // Controlar la bomba de agua
    if (humidity < 21) {
      digitalWrite(PUMP_PIN, HIGH);  // Encender la bomba de agua
      Serial.println("Bomba de agua encendida");
    } else {
      digitalWrite(PUMP_PIN, LOW);   // Apagar la bomba de agua
      Serial.println("Bomba de agua apagada");
    }
  }
}