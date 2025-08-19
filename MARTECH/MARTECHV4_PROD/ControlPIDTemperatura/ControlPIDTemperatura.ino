#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include "DHT.h"
#include <PID_v1.h>  // Librería PID

// Definiciones y constantes
#define DHTPin 0  // Pin del sensor de temperatura y humedad del aire (GPIO 0)
#define DHTTYPE DHT11
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

// Variables para el control PID
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;  // Ajusta según sea necesario
PID pidVentilador(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
PID pidPeltierFrio(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
PID pidPeltierCalor(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

unsigned long previousMillis = 0;

void setup() {
  dht.begin();  // Inicializa el sensor DHT
  Serial.begin(115200);
  setup_wifi();  // Conectar a la red WiFi
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(VENTILADOR_PIN, OUTPUT);
  pinMode(PELTIER_FRIO_PIN, OUTPUT);
  pinMode(PELTIER_CALOR_PIN, OUTPUT);
  
  // Inicializar PID
  pidVentilador.SetMode(AUTOMATIC);
  pidPeltierFrio.SetMode(AUTOMATIC);
  pidPeltierCalor.SetMode(AUTOMATIC);
  
  Setpoint = 22.0;  // Setpoint de temperatura deseada
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

// Función para conectar a la red WiFi
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Función de callback para mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Control manual del ventilador
  if (String(topic) == "control/manual/ventilador") {
    if (message == "on") {
      digitalWrite(VENTILADOR_PIN, HIGH);
    } else if (message == "off") {
      digitalWrite(VENTILADOR_PIN, LOW);
    }
  }

  // Control manual de las celdas Peltier
  if (String(topic) == "control/manual/peltier/frio") {
    if (message == "on") {
      digitalWrite(PELTIER_FRIO_PIN, HIGH);
    } else if (message == "off") {
      digitalWrite(PELTIER_FRIO_PIN, LOW);
    }
  }

  if (String(topic) == "control/manual/peltier/caliente") {
    if (message == "on") {
      digitalWrite(PELTIER_CALOR_PIN, HIGH);
    } else if (message == "off") {
      digitalWrite(PELTIER_CALOR_PIN, LOW);
    }
  }
}

// Función para reconectar al servidor MQTT
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe("control/manual/ventilador");
      client.subscribe("control/manual/peltier/frio");
      client.subscribe("control/manual/peltier/caliente");
    } else {
      delay(5000);
    }
  }
}

// Función principal del programa
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 2000) {
    previousMillis = currentMillis;

    // Leer temperatura del sensor DHT11
    float temperature = dht.readTemperature();
    
    if (isnan(temperature)) {
      return;
    }

    // Control PID de los actuadores
    Input = temperature;
    
    pidVentilador.Compute();
    analogWrite(VENTILADOR_PIN, Output);
    
    pidPeltierFrio.Compute();
    analogWrite(PELTIER_FRIO_PIN, Output);
    
    pidPeltierCalor.Compute();
    analogWrite(PELTIER_CALOR_PIN, Output);

    // Mostrar temperatura en LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + String(temperature) + "C");
    
    // Publicar temperatura en el servidor MQTT
    client.publish("sensor/temperatura", String(temperature).c_str());
  }
}
