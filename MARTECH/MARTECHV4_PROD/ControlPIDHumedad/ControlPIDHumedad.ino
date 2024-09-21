/*
Este código realiza el control de la humedad de un ambiente cerrado
utilizando un sensor DHT para medir la humedad y un sistema PID para
ajustar el funcionamiento de un humidificador y un extractor. 
El sistema se comunica a través de MQTT para monitorizar el estado y
los valores de humedad.
El sensor DHT mide la humedad del aire.
El control PID ajusta la salida para activar o desactivar el 
humidificador y el extractor en función de la humedad medida y la deseada.
Se utiliza MQTT para publicar el valor de humedad actual a un servidor, 
lo que permite la supervisión remota.
*/
#include <DHT.h>
#include <PID_v1.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Pines y configuración del sensor DHT
#define DHTPIN D4     // Pin de datos del sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor: DHT11 o DHT22
DHT dht(DHTPIN, DHTTYPE);

// Variables para el control PID de la humedad
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1; // Constantes PID
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Pines de control del humidificador y extractor
const int humidifierPin = D1;
const int extractorPin = D2;

// Variables para MQTT
const char* ssid = "tu_red_wifi";
const char* password = "tu_contraseña_wifi";
const char* mqtt_server = "broker.mqtt.com";
WiFiClient espClient;
PubSubClient client(espClient);

// Función de conexión al WiFi
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Función para controlar el humidificador y el extractor
void controlHumidity() {
  if (Output > 0) {
    digitalWrite(humidifierPin, HIGH);  // Activa el humidificador
    digitalWrite(extractorPin, LOW);    // Apaga el extractor
  } else {
    digitalWrite(humidifierPin, LOW);   // Apaga el humidificador
    digitalWrite(extractorPin, HIGH);   // Activa el extractor
  }
}

// Función de configuración inicial
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Configuración de pines
  pinMode(humidifierPin, OUTPUT);
  pinMode(extractorPin, OUTPUT);
  
  // Configuración del PID
  Setpoint = 90.0; // Humedad deseada en porcentaje
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255); // Limita la salida del PID
  
  // Configuración WiFi y MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  // Configurar función de callback
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

  // Control manual y automático de extractor
  if (String(topic) == "control/manual/extractor" || String(topic) == "control/automatico/extractor") {
    if (message == "on") {
      digitalWrite(extractorPin, HIGH);  // Encender extractor
      Serial.println("Extractor encendido");
    } else if (message == "off") {
      digitalWrite(extractorPin, LOW);  // Apagar extractor
      Serial.println("Extractor apagado");
    }
  }

  // Control manual y automático del humidificador
  if (String(topic) == "control/manual/humidificador" || String(topic) == "control/automatico/humidificador") {
    if (message == "on") {
      digitalWrite(humidifierPin, HIGH);  // Encender humidificador
      Serial.println("Humidificador encendidos");
    } else if (message == "off") {
      digitalWrite(humidifierPin, LOW);  // Apagar humidificador
      Serial.println("Humidificador apagado");
    }
  }
}

// Función principal
void loop() {
  // Leer la humedad actual
  Input = dht.readHumidity();

  // Controlar el PID
  myPID.Compute();
  
  // Controlar el sistema según la salida del PID
  controlHumidity();
  
  // Publicar datos en el servidor MQTT
  if (!client.connected()) {
    setup_wifi();
  }
  client.loop();
  String humidityStr = String(Input);
  client.publish("control/humedad", humidityStr.c_str());

  delay(2000); // Pausa entre lecturas
}