#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "ASP"; // Reemplaza con tu SSID
const char* password = "ASP110110"; // Reemplaza con tu contraseña
const char* mqtt_server = "192.168.1.12"; // Reemplaza con la IP o dirección de tu servidor MQTT

WiFiClient espClient;
PubSubClient client(espClient);

const int pinLDR = A0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int valorLDR = analogRead(pinLDR);
  String payload = String(valorLDR);
  client.publish("luz/sensorldr", (char*) payload.c_str());
  Serial.println("");
  Serial.println(valorLDR);

  delay(500); // Enviar cada medio segundo
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe("sensor/ldr");
    } else {
      delay(5000);
    }
  }
}