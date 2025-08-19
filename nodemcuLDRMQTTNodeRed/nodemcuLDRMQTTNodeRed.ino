// Librerías necesarias para la funcionalidad del ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Configuración de la red y el servidor MQTT ---

// Reemplaza con el nombre de tu red WiFi
const char* ssid = "ASP"; 
// Reemplaza con la contraseña de tu red WiFi
const char* password = "asp110110"; 
// Reemplaza con la dirección IP o dominio de tu servidor MQTT
const char* mqtt_server = "192.168.1.13"; 

// --- Variables globales y de estado ---

// Cliente WiFi para la conexión del ESP8266
WiFiClient espClient;
// Cliente MQTT que utiliza el cliente WiFi
PubSubClient client(espClient);

// Pin analógico donde está conectado el sensor LDR
const int pinLDR = A0;

// Variables para el control de tiempo no bloqueante
unsigned long lastMsg = 0;
// Intervalo de tiempo para enviar datos (en milisegundos)
const int interval = 500; 

// --- Funciones principales ---

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
    Serial.println("Conexión MQTT perdida. Reconectando...");
    // Intentar conectar con un ID de cliente único
    // Puedes cambiar "ESP8266Client" por algo más específico
    if (client.connect("ESP8266Client")) {
      Serial.println("¡Reconectado a MQTT!");
      // Opcional: suscribe un tópico al reconectar
      //client.subscribe("sensor/ldr");
    } else {
      Serial.print("Fallo en la reconexión. Código de error: ");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5 segundos...");
      // Espera 5 segundos antes de reintentar
      delay(5000);
    }
  }
}

// --- Rutinas de configuración (setup) y bucle principal (loop) ---

void setup() {
  // Inicia la comunicación serial para depuración
  Serial.begin(115200);
  
  // Llama a la función para conectar a WiFi
  setup_wifi();
  
  // Establece la dirección del servidor MQTT y el puerto (1883 es el puerto por defecto)
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Asegura que el cliente MQTT se mantenga conectado
  if (!client.connected()) {
    reconnect();
  }
  // Permite que el cliente MQTT procese mensajes entrantes y mantenga la conexión
  client.loop();

  // Control de tiempo para envío de mensajes no bloqueante
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Lectura del valor analógico del sensor LDR
    int valorLDR = analogRead(pinLDR);
    
    // Convierte el valor a una cadena de texto para enviar
    String payload = String(valorLDR);

    // Publica el valor en el tópico "luz/sensorldr"
    // El .c_str() convierte la String de C++ a una cadena de caracteres estilo C
    client.publish("luz/sensorldr", payload.c_str());

    // Imprime el valor en el monitor serial para depuración
    Serial.print("Publicando valor LDR: ");
    Serial.println(valorLDR);
  }
}
