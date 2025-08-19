#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include "DHT.h"

// Definiciones y constantes
#define DHTTYPE DHT11
#define DHTPin 0 // Pin del sensor de temperatura y humedad del aire (GPIO 0)
#define SDA_PIN 4  // Pin SDA para I2C (GPIO 4)
#define SCL_PIN 5  // Pin SCL para I2C (GPIO 5)
#define HUMIDIFICADOR_PIN 12 // Pin del humidificador (GPIO 12)
#define EXTRACTOR_PIN 13 // Pin del extractor (GPIO 13)

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

// Inicialización de LCD
LiquidCrystal_I2C_Hangul lcd(0x27, 16, 2);

unsigned long previousMillis = 0;  // Variable para controlar el tiempo

void setup() {
  dht.begin();  // Inicializa el sensor DHT
  Serial.begin(115200);
  setup_wifi();  // Conectar a la red WiFi
  client.setServer(mqtt_server, 1883);  // Configurar servidor MQTT
  client.setCallback(callback);  // Configurar función de callback
  pinMode(HUMIDIFICADOR_PIN, OUTPUT);  // Configurar pin del humidificador como salida
  pinMode(EXTRACTOR_PIN, OUTPUT);  // Configurar pin de los coolers como salida
  digitalWrite(HUMIDIFICADOR_PIN, LOW);  // Asegurarse de que el humidificador esté apagado
  digitalWrite(EXTRACTOR_PIN, LOW);  // Asegurarse de que los coolers estén apagados
    
  // Iniciar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Iniciar LCD
  lcd.init();
  lcd.backlight();
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

  // Control manual y automático del extractor
  if (String(topic) == "control/manual/extractor" || String(topic) == "control/automatico/extractor") {
    if (message == "on") {
      digitalWrite(EXTRACTOR_PIN, HIGH);  // Encender extractor
      Serial.println(" encendidos");
    } else if (message == "off") {
      digitalWrite(EXTRACTOR_PIN, LOW);  // Apagar extractor
      Serial.println("Extractor apagado");
    }
  }

  // Control manual y automático del humidificador
  if (String(topic) == "control/manual/humidificador" || String(topic) == "control/automatico/humidificador") {
    if (message == "on") {
      digitalWrite(HUMIDIFICADOR_PIN, HIGH);  // Encender humidificador
      Serial.println("Humidificador encendido");
    } else if (message == "off") {
      digitalWrite(HUMIDIFICADOR_PIN, LOW);  // Apagar humidificador
      Serial.println("Humidificador apagado");
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
      client.subscribe("control/manual/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/manual/extractor"); // Suscribirse al tema para el control del extractor
      client.subscribe("control/automatico/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/automatico/extractor"); // Suscribirse al tema para el control del extractor
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
  // Publicar humedad cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();

    // Verificar si las lecturas son válidas
    if (isnan(humidity)) {
      Serial.println("Fallo en la lectura del sensor DHT");
      return;
    }

    // Publicar valores de humedad
    client.publish("humedad/sensordht11", String(humidity).c_str());
    
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    // Mostrar valores en el LCD
    lcd.setCursor(0, 0);
    lcd.print("Humedad: ");
    lcd.print(humidity);
    lcd.print("%");
  }
}
