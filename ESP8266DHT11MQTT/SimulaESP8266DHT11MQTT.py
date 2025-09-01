# Importa las librerías necesarias
import paho.mqtt.client as mqtt
import time
import random

# --- Configuración del servidor MQTT ---

# Reemplaza con la dirección IP o dominio de tu servidor MQTT
MQTT_SERVER = "192.168.1.17"
# Puerto estándar de MQTT
MQTT_PORT = 1883
# Tópicos a los que se publicarán los datos del DHT11
MQTT_TOPIC_TEMP = "tempaire/sensordht11"
MQTT_TOPIC_HUM = "humidity/sensordht11"
# ID de cliente para el simulador
CLIENT_ID = "PythonSimDHT11"

# Crea una instancia del cliente MQTT
client = mqtt.Client(client_id=CLIENT_ID)

# --- Funciones del cliente MQTT ---

# Función que se ejecuta al conectar al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado exitosamente al broker MQTT.")
    else:
        print(f"Fallo en la conexión. Código de error: {rc}")

# Función que se ejecuta al desconectar del broker
def on_disconnect(client, userdata, rc):
    print("Desconectado del broker MQTT.")

# Asigna las funciones de callback
client.on_connect = on_connect
client.on_disconnect = on_disconnect

# --- Bucle de simulación ---

try:
    # Conecta al broker MQTT
    print(f"Intentando conectar a {MQTT_SERVER}:{MQTT_PORT}...")
    client.connect(MQTT_SERVER, MQTT_PORT, 60)
    # Inicia el bucle en segundo plano para procesar los mensajes
    client.loop_start()

    # Bucle principal para simular las lecturas y publicar los datos
    while True:
        # Simula una lectura de temperatura (ej. 20.0 a 30.0 °C)
        temperatura = round(random.uniform(20.0, 30.0), 2)
        # Simula una lectura de humedad (ej. 40.0 a 60.0 %)
        humedad = round(random.uniform(40.0, 60.0), 2)
        
        # Convierte los valores a cadenas de texto
        payload_temp = str(temperatura)
        payload_hum = str(humedad)

        # Publica el valor de la temperatura en su tópico
        client.publish(MQTT_TOPIC_TEMP, payload_temp, qos=1)
        print(f"Publicando en '{MQTT_TOPIC_TEMP}': {payload_temp}°C")

        # Publica el valor de la humedad en su tópico
        client.publish(MQTT_TOPIC_HUM, payload_hum, qos=1)
        print(f"Publicando en '{MQTT_TOPIC_HUM}': {payload_hum}%")

        # Espera 10 segundos antes de la siguiente lectura (como en el ESP8266)
        time.sleep(10)

except KeyboardInterrupt:
    print("Simulación terminada por el usuario.")
except Exception as e:
    print(f"Ocurrió un error: {e}")
finally:
    # Detiene el bucle y se desconecta del broker
    client.loop_stop()
    client.disconnect()
