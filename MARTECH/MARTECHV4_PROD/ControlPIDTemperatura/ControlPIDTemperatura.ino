/*
Este código mantiene únicamente el control de la temperatura
con el sensor DHT11, utilizando PID para regular los coolers
y el Peltier, y publica la temperatura medida a través de MQTT
en el tema "sensores/temperatura".
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"
#include <PID_v1.h>

// Definiciones y constantes
#define LDR_PIN A0  // Pin del sensor de luz
#define LED_PIN LED_BUILTIN  // LED interno del ESP8266
#define DHTTYPE DHT11
#define DHTPin 0 // Pin del sensor de temperatura y humedad del aire (GPIO 0)
#define SDA_PIN 4  // Pin SDA para I2C (GPIO 4)
#define SCL_PIN 5  // Pin SCL para I2C (GPIO 5)
#define PUMP_PIN 12 // Pin del humidificador (GPIO 12)
#define COOLERS_PIN 13 // Pin de los coolers (GPIO 13)
#define PELTIER_PIN 14 // Pin del Peltier (GPIO 14)
#define MQ2_PIN A0  // Pin del sensor MQ-2 (GPIO A0)

// Inicialización del sensor DHT
DHT dht(DHTPin, DHTTYPE);

// Credenciales WiFi
const char* ssid = "AndroidAP";
const char* password = "wrkn9541";

// Servidor MQTT
const char* mqtt_server = "192.168.43.31";

// Credenciales MQTT
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Inicialización de clientes WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Variables para el control PID
double temperatureSetpoint = 21.0; // Setpoint para temperatura (rango de 18-24 °C)
double humiditySetpoint = 90.0;    // Setpoint para humedad (rango de 85-95%)
double airQualitySetpoint = 500.0; // Setpoint para calidad del aire (ppm)

double currentTemperature, pidOutputTemp;
double currentHumidity, pidOutputHumidity;
double currentAirQuality, pidOutputAirQuality;

PID tempPID(&currentTemperature, &pidOutputTemp, &temperatureSetpoint, 2, 5, 1, DIRECT);
PID humidityPID(&currentHumidity, &pidOutputHumidity, &humiditySetpoint, 2, 5, 1, DIRECT);
PID airQualityPID(&currentAirQuality, &pidOutputAirQuality, &airQualitySetpoint, 2, 5, 1, DIRECT);

unsigned long previousMillis = 0;  // Variable para controlar el tiempo

void setup() {
  dht.begin();  // Inicializa el sensor DHT
  Serial.begin(115200);
  setup_wifi();  // Conectar a la red WiFi
  client.setServer(mqtt_server, 1883);  // Configurar servidor MQTT
  client.setCallback(callback);  // Configurar función de callback
  pinMode(LED_PIN, OUTPUT);   // Configurar pin del LED interno como salida
  pinMode(PUMP_PIN, OUTPUT);  // Configurar pin del humidificador como salida
  pinMode(COOLERS_PIN, OUTPUT);  // Configurar pin de los coolers como salida
  pinMode(PELTIER_PIN, OUTPUT);  // Configurar pin del Peltier como salida
  digitalWrite(LED_PIN, HIGH);  // Asegurarse de que el LED esté apagado inicialmente (LED_BUILTIN está invertido)
  digitalWrite(PUMP_PIN, LOW);  // Asegurarse de que el humidificador esté apagado inicialmente
  digitalWrite(COOLERS_PIN, LOW);  // Asegurarse de que los coolers estén apagados inicialmente
  digitalWrite(PELTIER_PIN, LOW);  // Asegurarse de que el Peltier esté apagado inicialmente

  // Inicialización de PID
  tempPID.SetMode(AUTOMATIC);
  humidityPID.SetMode(AUTOMATIC);
  airQualityPID.SetMode(AUTOMATIC);
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
  if (String(topic) == "control/manual/led") {
    if (message == "on") {
      digitalWrite(LED_PIN, LOW);  // Encender el LED (LED_BUILTIN está invertido)
      Serial.println("LED encendido");
    } else if (message == "off") {
      digitalWrite(LED_PIN, HIGH);  // Apagar el LED (LED_BUILTIN está invertido)
      Serial.println("LED apagado");
    }
  }

  // Control manual y automático de los coolers
  if (String(topic) == "control/manual/coolers" || String(topic) == "control/automatico/coolers") {
    if (message == "on") {
      digitalWrite(COOLERS_PIN, HIGH);  // Encender coolers
      Serial.println("Coolers encendidos");
    } else if (message == "off") {
      digitalWrite(COOLERS_PIN, LOW);  // Apagar coolers
      Serial.println("Coolers apagados");
    }
  }

  // Control manual y automático del humidificador
  if (String(topic) == "control/manual/humidificador" || String(topic) == "control/automatico/humidificador") {
    if (message == "on") {
      digitalWrite(PUMP_PIN, HIGH);  // Encender humidificador
      Serial.println("Humidificador encendidos");
    } else if (message == "off") {
      digitalWrite(PUMP_PIN, LOW);  // Apagar humidificador
      Serial.println("Humidificador apagado");
    }
  }

  // Control manual y automático del Peltier
  if (String(topic) == "control/manual/peltier" || String(topic) == "control/automatico/peltier") {
    if (message == "on") {
      digitalWrite(PELTIER_PIN, HIGH);  // Encender Peltier
      Serial.println("Peltier encendido");
    } else if (message == "off") {
      digitalWrite(PELTIER_PIN, LOW);  // Apagar Peltier
      Serial.println("Peltier apagado");
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
      client.subscribe("control/manual/led");  // Suscribirse al tema para el control del LED
      client.subscribe("control/manual/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/manual/coolers"); // Suscribirse al tema para el control de coolers
      client.subscribe("control/manual/peltier"); // Suscribirse al tema para el control del Peltier
      client.subscribe("control/automatico/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/automatico/coolers"); // Suscribirse al tema para el control de coolers
      client.subscribe("control/automatico/peltier"); // Suscribirse al tema para el control del Peltier
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
  // Publicar temperatura, humedad, luz y gas cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    int ldrValue = analogRead(LDR_PIN);  // Leer valor del LDR
    int mq2Value = analogRead(MQ2_PIN);  // Leer valor del MQ-2

    // Verificar si las lecturas son válidas
    if (isnan(humidity) || isnan(temperatureC)) {
      Serial.println("Fallo en la lectura del sensor DHT");
      return;
    }

    // Control PID para temperatura, humedad y calidad del aire
    currentTemperature = temperatureC;
    currentHumidity = humidity;
    currentAirQuality = mq2Value;

    tempPID.Compute();
    humidityPID.Compute();
    airQualityPID.Compute();

    // Control de dispositivos basado en PID
    digitalWrite(COOLERS_PIN, pidOutputTemp > 0 ? HIGH : LOW);
    digitalWrite(PUMP_PIN, pidOutputHumidity > 0 ? HIGH : LOW);
    digitalWrite(PELTIER_PIN, pidOutputAirQuality > 0 ? HIGH : LOW);

    // Publicar valores a través de MQTT
    String tempStr = "Temperatura: " + String(temperatureC) + "C";
    String humStr = "Humedad: " + String(humidity) + "%";
    String lightStr = "Luz: " + String(ldrValue);
    String airQualityStr = "Calidad del aire: " + String(mq2Value) + " ppm";

    Serial.println(tempStr);
    Serial.println(humStr);
    Serial.println(lightStr);
    Serial.println(airQualityStr);

    client.publish("sensores/temperatura", String(temperatureC).c_str());
    client.publish("sensores/humedad", String(humidity).c_str());
    client.publish("sensores/luz", String(ldrValue).c_str());
    client.publish("sensores/aire", String(mq2Value).c_str());
  }
}
