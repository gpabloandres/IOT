#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include "DHT.h"

// Definiciones y constantes
#define DHTTYPE DHT11
#define DHTPin 0 // Pin del sensor de temperatura y humedad (GPIO 0)
#define VENTILADOR_PIN 14 // Pin del ventilador (GPIO 14)
#define PELTIER_FRIO_PIN 15 // Pin para la celda de Peltier de frío (GPIO 15)
#define PELTIER_CALOR_PIN 16 // Pin para la celda de Peltier de calor (GPIO 16)

// Inicialización del sensor DHT
DHT dht(DHTPin, DHTTYPE);

// Credenciales WiFi
const char* ssid = "AndroidAP";
const char* password = "wrkn9541";

// Servidor MQTT
const char* mqtt_server = "192.168.43.31";

// Inicialización de clientes WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Inicialización de LCD
LiquidCrystal_I2C_Hangul lcd(0x27, 16, 2);

unsigned long previousMillis = 0;

void setup() {
  dht.begin();  // Inicializa el sensor DHT
  Serial.begin(115200);
  setup_wifi();  // Conectar a la red WiFi
  client.setServer(mqtt_server, 1883);  // Configurar servidor MQTT
  client.setCallback(callback);  // Configurar función de callback
  pinMode(VENTILADOR_PIN, OUTPUT);  // Configurar pin del ventilador como salida
  pinMode(PELTIER_FRIO_PIN, OUTPUT);  // Configurar pin de la celda de frío como salida
  pinMode(PELTIER_CALOR_PIN, OUTPUT);  // Configurar pin de la celda de calor como salida
  digitalWrite(VENTILADOR_PIN, LOW);  // Asegurarse de que el ventilador esté apagado
  digitalWrite(PELTIER_FRIO_PIN, LOW);  // Apagar celda de frío
  digitalWrite(PELTIER_CALOR_PIN, LOW);  // Apagar celda de calor
  
  // Iniciar I2C
  Wire.begin(4, 5);
  
  // Iniciar LCD
  lcd.init();
  lcd.backlight();
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado a WiFi");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Control manual y automático del ventilador
  if (String(topic) == "control/manual/ventilador" || String(topic) == "control/automatico/ventilador") {
    if (message == "on") {
      digitalWrite(VENTILADOR_PIN, HIGH);  // Encender ventilador
      Serial.println("Ventilador encendido");
    } else if (message == "off") {
      digitalWrite(VENTILADOR_PIN, LOW);  // Apagar ventilador
      Serial.println("Ventilador apagado");
    }
  }
  
  // Control manual y automático del Peltier Frio
  if (String(topic) == "control/manual/peltier/frio" || String(topic) == "control/automatico/peltier/frio") {
    if (message == "on") {
      digitalWrite(PELTIER_FRIO_PIN, HIGH);  // Encender Peltier frío
      Serial.println("Peltier frío encendido");
    } else if (message == "off") {
      digitalWrite(PELTIER_FRIO_PIN, LOW);  // Apagar Peltier frío
      Serial.println("Peltier frío apagado");
    }
  }

  // Control manual y automático del Peltier Caliente
  if (String(topic) == "control/manual/peltier/caliente" || String(topic) == "control/automatico/peltier/caliente") {
    if (message == "on") {
      digitalWrite(PELTIER_CALOR_PIN, HIGH);  // Encender Peltier caliente
      Serial.println("Peltier calor encendido");
    } else if (message == "off") {
      digitalWrite(PELTIER_CALOR_PIN, LOW);  // Apagar Peltier caliente
      Serial.println("Peltier calor apagado");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe("control/manual/ventilador");
      client.subscribe("control/manual/peltier/frio");
      client.subscribe("control/manual/peltier/caliente");
      client.subscribe("control/automatico/ventilador");
      client.subscribe("control/automatico/peltier/frio");
      client.subscribe("control/automatico/peltier/caliente");
    } else {
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
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;
    float temperatureC = dht.readTemperature();
    
    if (!isnan(temperatureC)) {
      client.publish("tempaire/sensordht11", String(temperatureC).c_str());
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperatureC);
      lcd.print("C");
    }
  }
}
