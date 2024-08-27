#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include "DHT.h"

// Definiciones y constantes
#define LDR_PIN A0  // Pin del sensor de luz
#define LED_PIN LED_BUILTIN  // LED interno del ESP8266
#define DHTTYPE DHT11
#define DHTPin 0 // Pin del sensor de temperatura y humedad del aire (GPIO 0)
#define SDA_PIN 4  // Pin SDA para I2C (GPIO 4)
#define SCL_PIN 5  // Pin SCL para I2C (GPIO 5)
#define PUMP_PIN 12 // Pin del humidificador (GPIO 12)
#define COOLERS_PIN 13 // Pin de los coolers (GPIO 13)
#define PELTIER_PIN 14 // Pin de los coolers (GPIO 14)

// Inicialización del sensor DHT
DHT dht(DHTPin, DHTTYPE);

// Credenciales WiFi
const char* ssid = "AndroidAP";
const char* password = "wrkn9541";

// Servidor MQTT
const char* mqtt_server = "192.168.43.37";

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
  pinMode(LED_PIN, OUTPUT);   // Configurar pin del LED interno como salida
  pinMode(PUMP_PIN, OUTPUT);  // Configurar pin del humificador como salida
  pinMode(COOLERS_PIN, OUTPUT);  // Configurar pin de los coolers como salida
  pinMode(PELTIER_PIN, OUTPUT);  // Configurar pin del peltier como salida
  digitalWrite(LED_PIN, HIGH);  // Asegurarse de que el LED esté apagado inicialmente (LED_BUILTIN está invertido)
  digitalWrite(PUMP_PIN, LOW);  // Asegurarse de que el humificador esté apagado inicialmente
  digitalWrite(COOLERS_PIN, LOW);  // Asegurarse de que los coolers esten apagados inicialmente
  digitalWrite(PELTIER_PIN, LOW);  // Asegurarse de que el peltier esté apagado inicialmente
    
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
      digitalWrite(PELTIER_PIN, HIGH);  // Encender peltier
      Serial.println("Peltier encendido");
    } else if (message == "off") {
      digitalWrite(PELTIER_PIN, LOW);  // Apagar peltier
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
      client.subscribe("control/manual/humidificador"); // Suscribirse al tema para el control del humificador
      client.subscribe("control/manual/coolers"); // Suscribirse al tema para el control de coolers
      client.subscribe("control/manual/peltier"); // Suscribirse al tema para el control del peltier
      client.subscribe("control/automatico/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/automatico/coolers"); // Suscribirse al tema para el control del coolers
      client.subscribe("control/automatico/peltier"); // Suscribirse al tema para el control del peltier
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
  // Publicar temperatura, humedad y luz cada 10 segundos
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    float temperatureF = dht.readTemperature(true);
    int ldrValue = analogRead(LDR_PIN);  // Leer valor del LDR

    // Verificar si las lecturas son válidas
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Fallo en la lectura del sensor DHT");
      return;
    }

    // Publicar valores de temperatura, humedad y luz
    client.publish("tempaire/sensordht11", String(temperatureC).c_str());
    client.publish("humidity/sensordht11", String(humidity).c_str());
    client.publish("light/sensorldr", String(ldrValue).c_str());

    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
    Serial.print("Luz: ");
    Serial.println(ldrValue);

    // Mostrar valores en el LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T: ");
    lcd.print(temperatureC);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("H: ");
    lcd.print(humidity);
    lcd.print(" %");
    lcd.print("L: ");
    lcd.print(ldrValue);
    lcd.print(" L");
  }
}
