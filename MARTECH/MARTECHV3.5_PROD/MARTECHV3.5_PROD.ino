#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include "DHT.h"

// Definiciones y constantes
//#define LDR_PIN A0  // Pin del sensor de luz
#define LED_PIN LED_BUILTIN  // LED interno del ESP8266
#define DHTTYPE DHT11
#define DHTPin 0 // Pin del sensor de temperatura y humedad del aire (GPIO 0)
#define SDA_PIN 4  // Pin SDA para I2C (GPIO 4)
#define SCL_PIN 5  // Pin SCL para I2C (GPIO 5)
#define HUMIDIFICADOR_PIN 12 // Pin del humidificador (GPIO 12)
#define EXTRACTOR_PIN 13 // Pin del extractor (GPIO 13)
#define VENTILADOR_PIN 14 // Pin del ventilador (GPIO 14)
#define PELTIER_FRIO_PIN 15 // Pin para la celda de Peltier de frío (GPIO 15)
#define PELTIER_CALOR_PIN 16 // Pin para la celda de Peltier de calor (GPIO 16)
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
  pinMode(HUMIDIFICADOR_PIN, OUTPUT);  // Configurar pin del humidificador como salida
  pinMode(EXTRACTOR_PIN, OUTPUT);  // Configurar pin de los coolers como salida
  pinMode(VENTILADOR_PIN, OUTPUT);  // Configurar pin del ventilador como salida
  pinMode(PELTIER_FRIO_PIN, OUTPUT);  // Configurar pin de la celda de frío como salida
  pinMode(PELTIER_CALOR_PIN, OUTPUT);  // Configurar pin de la celda de calor como salida
  digitalWrite(LED_PIN, HIGH);  // Asegurarse de que el LED esté apagado inicialmente
  digitalWrite(HUMIDIFICADOR_PIN, LOW);  // Asegurarse de que el humidificador esté apagado
  digitalWrite(EXTRACTOR_PIN, LOW);  // Asegurarse de que los coolers estén apagados
  digitalWrite(VENTILADOR_PIN, LOW);  // Asegurarse de que el ventilador esté apagado
  digitalWrite(PELTIER_FRIO_PIN, LOW);  // Apagar celda de frío
  digitalWrite(PELTIER_CALOR_PIN, LOW);  // Apagar celda de calor
    
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
      digitalWrite(PELTIER_FRIO_PIN, HIGH);  // Encender Peltier frio
      Serial.println("Peltier frio encendido");
    } else if (message == "off") {
      digitalWrite(PELTIER_FRIO_PIN, LOW);  // Apagar Peltier frio
      Serial.println("Peltier frio apagado");
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
      client.subscribe("control/manual/extractor"); // Suscribirse al tema para el control del extractor
      client.subscribe("control/manual/ventilador"); // Suscribirse al tema para el control del ventilador
      client.subscribe("control/manual/peltier/frio"); // Suscribirse al tema para el control del Peltier frío
      client.subscribe("control/manual/peltier/caliente"); // Suscribirse al tema para el control del Peltier caliente
      client.subscribe("control/automatico/humidificador"); // Suscribirse al tema para el control del humidificador
      client.subscribe("control/automatico/extractor"); // Suscribirse al tema para el control del extractor
      client.subscribe("control/automatico/ventilador"); // Suscribirse al tema para el control del ventilador
      client.subscribe("control/automatico/peltier/frio"); // Suscribirse al tema para el control del Peltier frío
      client.subscribe("control/automatico/peltier/caliente"); // Suscribirse al tema para el control del Peltier caliente
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
    float temperatureF = dht.readTemperature(true);
    //int ldrValue = analogRead(LDR_PIN);  // Leer valor del LDR
    int mq2Value = analogRead(MQ2_PIN);  // Leer valor del MQ-2

    // Verificar si las lecturas son válidas
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Fallo en la lectura del sensor DHT");
      return;
    }

    // Publicar valores de temperatura, humedad, luz y gas
    client.publish("tempaire/sensordht11", String(temperatureC).c_str());
    client.publish("humedad/sensordht11", String(humidity).c_str());
    //client.publish("light/sensorldr", String(ldrValue).c_str());
    client.publish("gas/sensormq2", String(mq2Value).c_str());

    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
    //Serial.print("Luz: ");
    //Serial.println(ldrValue);
    Serial.print("Gas (MQ-2): ");
    Serial.println(mq2Value);
    Serial.println(" ppm");

    // Mostrar valores en el LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatureC);
    lcd.print("C");
    lcd.setCursor(0, 1);
    //lcd.print("Luz: ");
    //lcd.print(ldrValue);
    lcd.print(" Gas: ");
    lcd.print(mq2Value);
  }
}
