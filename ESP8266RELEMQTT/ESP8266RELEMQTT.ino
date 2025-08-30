// Librerías necesarias para la funcionalidad del ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Configuración de la red y el servidor MQTT ---

// Reemplaza con el nombre de tu red WiFi
const char* ssid = "MOVISTAR WIFI4497";
// Reemplaza con la contraseña de tu red WiFi
const char* password = "689ms4283kxz7ge";
// Reemplaza con la dirección IP o dominio de tu servidor MQTT
const char* mqtt_server = "10.50.139.113";

// Cliente WiFi para la conexión del ESP8266
WiFiClient espClient;
// Cliente MQTT que utiliza el cliente WiFi
PubSubClient client(espClient);

// Define el pin donde está conectado el relé.
// En muchas placas NodeMCU ESP8266, D1 corresponde a GPIO5.
const int relePin = D1;

// --- Funciones principales ---

// Maneja los mensajes entrantes del servidor MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Imprime el tópico y el mensaje recibido en el monitor serial
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Compara el primer carácter del mensaje
  if ((char)payload[0] == '1') {
    // Si el mensaje es '1', activa el relé (lógica activa en bajo)
    // Esto significa que el pin de control se pone en LOW
    digitalWrite(relePin, LOW);
    Serial.println("Relé ACTIVADO.");
  } else if ((char)payload[0] == '0') {
    // Si el mensaje es '0', desactiva el relé (lógica activa en bajo)
    // Esto significa que el pin de control se pone en HIGH
    digitalWrite(relePin, HIGH);
    Serial.println("Relé DESACTIVADO.");
  }
}

// Configura y establece la conexión WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a la red ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Bucle de espera hasta que la conexión WiFi sea exitosa
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("¡Conectado a la red WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Se ejecuta si la conexión MQTT se pierde para intentar reconectar
void reconnect() {
  // Bucle para intentar reconectar
  while (!client.connected()) {
    Serial.println("Conexión MQTT perdida. Intentando reconectar...");
    
    // Intenta conectar con un ID de cliente único
    if (client.connect("ESP8266_Rele_Client")) {
      Serial.println("¡Reconectado a MQTT!");
      // Se suscribe al tópico para recibir comandos del relé
      client.subscribe("control/rele");
      Serial.println("Suscrito al tópico 'control/rele'");
    } else {
      Serial.print("Fallo en la reconexión. Código de estado: ");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5 segundos...");
      // Espera 5 segundos antes de reintentar
      delay(5000);
    }
  }
}

// --- Rutinas de configuración (setup) y bucle principal (loop) ---

void setup() {
  // Configura el pin del relé como salida
  pinMode(relePin, OUTPUT);
  // Por defecto, se recomienda que el relé esté desactivado al inicio
  digitalWrite(relePin, HIGH); 
  
  // Inicia la comunicación serial para depuración
  Serial.begin(115200);
  
  // Llama a la función para conectar a WiFi
  setup_wifi();
  
  // Establece la dirección del servidor MQTT y el puerto (1883 es el puerto por defecto)
  client.setServer(mqtt_server, 1883);
  // Asigna la función de callback para manejar los mensajes MQTT
  client.setCallback(callback);
}

void loop() {
  // Asegura que el cliente MQTT se mantenga conectado
  if (!client.connected()) {
    reconnect();
  }
  // Permite que el cliente MQTT procese mensajes entrantes y mantenga la conexión
  client.loop();
}
