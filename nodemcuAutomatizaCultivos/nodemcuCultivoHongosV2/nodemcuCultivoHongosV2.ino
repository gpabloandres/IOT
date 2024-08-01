#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Definiciones y constantes
#define DHTTYPE DHT11
#define DHTPin 4  // Pin sensor de temperatura y humedad aire
#define PUMP_PIN 0  // Pin de la bomba de agua
#define LED_PIN LED_BUILTIN  // LED interno del ESP8266

DHT dht(DHTPin, DHTTYPE);

// Credenciales WiFi
const char* ssid = "ASP";
const char* password = "ASP110110";

// Servidor MQTT
const char* mqtt_server = "192.168.1.17";

// Credenciales MQTT
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Inicialización de clientes WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;  // Variable para controlar el tiempo

void setup() {
  dht.begin();  // Inicializa el sensor DHT
  Serial.begin(115200);
  setup_wifi();  // Conectar a la red WiFi
  client.setServer(mqtt_server, 1883);  // Configurar servidor MQTT
  client.setCallback(callback);  // Configurar función de callback
  pinMode(PUMP_PIN, OUTPUT);  // Configurar pin de la bomba como salida
  pinMode(LED_PIN, OUTPUT);   // Configurar pin del LED interno como salida
  digitalWrite(PUMP_PIN, LOW);  // Asegurarse de que la bomba esté apagada inicialmente
  digitalWrite(LED_PIN, HIGH);  // Asegurarse de que el LED esté apagado inicialmente (LED_BUILTIN está invertido)
}

// Función para conectar a la red WiFi
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
  Serial.print("Conectado a WiFi - Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función de callback para mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Controlar el LED interno
  if (String(topic) == "control/led") {
    if (message == "on") {
      digitalWrite(LED_PIN, LOW);  // Encender el LED (LED_BUILTIN está invertido)
      Serial.println("LED encendido");
    } else if (message == "off") {
      digitalWrite(LED_PIN, HIGH);  // Apagar el LED (LED_BUILTIN está invertido)
      Serial.println("LED apagado");
    }
  }

  // Controlar la bomba de agua
  if (String(topic) == "control/pump") {
    if (message == "on") {
      digitalWrite(PUMP_PIN, HIGH);  // Encender la bomba
      Serial.println("Bomba de agua encendida");
    } else if (message == "off") {
      digitalWrite(PUMP_PIN, LOW);  // Apagar la bomba
      Serial.println("Bomba de agua apagada");
    }
  }
}

// Función para reconectar al servidor MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("conectado");  
      client.publish("event", "hello world");  // Publicar un mensaje
      client.subscribe("event");  // Suscribirse a un tema
      client.subscribe("control/led");  // Suscribirse al tema para el control del LED
      client.subscribe("control/pump"); // Suscribirse al tema para el control de la bomba
    } else {
      Serial.print("fallo, rc=");
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
  if (!client.loop()) {
    client.connect("NodeMCUClient", MQTT_username, MQTT_password);
  }

  unsigned long currentMillis = millis();
  // Publicar temperatura y humedad cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    float temperatureF = dht.readTemperature(true);

    // Verificar si las lecturas son válidas
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Fallo en la lectura del sensor DHT");
      return;
    }

    // Publicar valores de temperatura y humedad
    client.publish("tempaire/sensordht11", String(temperatureC).c_str());
    client.publish("humidity/sensordht11", String(humidity).c_str());

    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");

    // Controlar la bomba de agua según la humedad
    if (humidity < 29) {
      digitalWrite(PUMP_PIN, HIGH);  // Encender la bomba
      Serial.println("Bomba de agua encendida");
    } else {
      digitalWrite(PUMP_PIN, LOW);  // Apagar la bomba
      Serial.println("Bomba de agua apagada");
    }
  }
}
