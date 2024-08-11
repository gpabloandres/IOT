#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DIYables_IRcontroller.h> // Librería DIYables_IRcontroller

#define IR_RECEIVER_PIN D4 // Pin del ESP8266 conectado al receptor IR

// Configuración WiFi
const char* ssid = "ASP"; // Cambia esto por tu SSID
const char* password = "ASP110110"; // Cambia esto por tu contraseña WiFi

// Configuración MQTT
const char* mqtt_server = "192.168.1.17"; // Cambia esto por la dirección IP de tu broker MQTT
const char* mqtt_topic = "ir/code"; // Tema donde se publicará el código IR
WiFiClient espClient;
PubSubClient client(espClient);

DIYables_IRcontroller_21 irController(IR_RECEIVER_PIN, 200); // Tiempo de rebote configurado en 200ms

void setup_wifi() {
  delay(10);
  // Conexión a la red WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Bucle hasta que se conecte a MQTT
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Intenta conectarse
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("fallido, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  irController.begin();
  
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Configura el servidor MQTT y el puerto (1883 por defecto)
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Key21 command = irController.getKey();
  if (command != Key21::NONE) {
    char message[10];
    sprintf(message, "%X", static_cast<int>(command)); // Convierte a hexadecimal y almacena en message
    Serial.print("Código IR capturado: ");
    Serial.println(message);
    
    // Publica el mensaje en el tema MQTT
    client.publish(mqtt_topic, message);
    delay(1000); // Pausa para evitar lecturas repetidas del mismo comando
  }
}
