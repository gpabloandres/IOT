#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h> 

// parametros wifi
const char* ssid = "ASP";
const char* password = "ASP110110";

// el 2 es el led de la placa
int ledOnBoard = 2;

// parametros mqtt
const char* mqtt_server = "192.168.1.17";
WiFiClient espClient;
PubSubClient client(espClient);

// declaro el servomotor
Servo myservo; 

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledOnBoard, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  myservo.attach(13);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(ledOnBoard, HIGH);
    delay(500);
    digitalWrite(ledOnBoard, LOW);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char msg[length + 1];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg[i] = (char)payload[i];
  }
  msg[length] = '\0';
  Serial.println();

  if (strcmp(topic, "servo") == 0) {
    int resultado = atoi(msg);
    int pos = map(resultado, 1, 100, 0, 180);
    Serial.print("Moving servo to position: ");
    Serial.println(pos);
    myservo.write(pos);
    delay(250);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {

      Serial.println("connected");
      client.subscribe("servo");
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
  client.loop();

 delay(100);
}