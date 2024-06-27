#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPin 4

DHT dht(DHTPin, DHTTYPE);

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "ASP";
const char* password = "ASP110110";
const char* mqtt_server = "192.168.1.17";     /// MQTT  Broker

// MQTT broker credentials (set to NULL if not required)
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;

void setup() {
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Función para reconectar al servidor MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //With User and Passwordif (client.connect("NodeMCUClient", MQTT_username, MQTT_password)) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Once connected, publish an announcement...
      client.publish("event", "hello world");
      // ... and resubscribe
      client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NodeMCUClient", MQTT_username, MQTT_password);
  
  unsigned long currentMillis = millis();
  // Publishes new temperature and humidity every 10 seconds
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temperatureC = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float temperatureF = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Publishes Temperature and Humidity values
    client.publish("tempaire/sensordht11", String(temperatureC).c_str());
    client.publish("humidity/sensordht11", String(humidity).c_str());
    //Uncomment to publish temperature in F degrees
    //client.publish("iotfrontier/temperature", String(temperatureF).c_str());
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
    //Serial.print(temperatureF);
    //Serial.println(" ºF");
  }
}