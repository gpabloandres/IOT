// Librerías necesarias para la funcionalidad del ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Configuración de la red y el servidor MQTT ---

// Reemplaza con el nombre de tu red WiFi
const char* ssid = "ASP";
// Reemplaza con la contraseña de tu red WiFi
const char* password = "ASP110110";
// Reemplaza con la dirección IP o dominio de tu servidor MQTT
const char* mqtt_server = "192.168.1.17";

// Cliente WiFi para la conexión del ESP8266
WiFiClient espClient;
// Cliente MQTT que utiliza el cliente WiFi
PubSubClient client(espClient);

// Define el pin del LED integrado.
// En muchas placas ESP8266, BUILTIN_LED está conectado a GPIO2.
// Ten en cuenta que es activo-bajo, es decir, LOW lo enciende y HIGH lo apaga.
const int ledPin = BUILTIN_LED;

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
    // Si el mensaje es '1', enciende el LED
    // BUILTIN_LED es activo-bajo, por eso se usa LOW
    digitalWrite(ledPin, LOW);
    Serial.println("LED encendido.");
  } else if ((char)payload[0] == '0') {
    // Si el mensaje es '0', apaga el LED
    // BUILTIN_LED es activo-bajo, por eso se usa HIGH
    digitalWrite(ledPin, HIGH);
    Serial.println("LED apagado.");
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
    if (client.connect("ESP8266_Led_Client")) {
      Serial.println("¡Reconectado a MQTT!");
      // Se suscribe al tópico para recibir comandos de LED
      client.subscribe("control/led");
      Serial.println("Suscrito al tópico 'control/led'");
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
  // Configura el pin del LED como salida
  pinMode(ledPin, OUTPUT);
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

  // El microcontrolador está ahora en un bucle de espera, listo para recibir comandos MQTT
}
