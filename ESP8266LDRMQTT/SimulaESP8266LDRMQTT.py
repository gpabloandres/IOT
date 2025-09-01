# Importa las librerías necesarias
import paho.mqtt.client as mqtt
import time
import random

# --- Configuración del servidor MQTT ---

# Reemplaza con la dirección IP o dominio de tu servidor MQTT
MQTT_SERVER = "10.0.20.30"
# Puerto estándar de MQTT
MQTT_PORT = 1883
# Tópico al que se publicarán los datos del LDR
MQTT_TOPIC = "luz/sensorldr"
# ID de cliente para el simulador
CLIENT_ID = "PythonSimLDR"

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
        # Simula una lectura del sensor LDR con un valor aleatorio
        # Rango de valores (25 a 900) para simular diferentes condiciones de luz
        valor_ldr = random.randint(25, 900)
        
        # Convierte el valor a una cadena de texto para publicarlo
        payload = str(valor_ldr)

        # Publica el valor en el tópico especificado
        result = client.publish(MQTT_TOPIC, payload, qos=1)
        # Confirma si el mensaje se publicó correctamente
        print(f"Publicando en el tópico '{MQTT_TOPIC}': {payload}")
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print("Mensaje publicado con éxito.")
        else:
            print(f"Error al publicar mensaje. Código: {result.rc}")

        # Espera 2 segundos antes de la siguiente lectura
        time.sleep(2)

except KeyboardInterrupt:
    print("Simulación terminada por el usuario.")
except Exception as e:
    print(f"Ocurrió un error: {e}")
finally:
    # Detiene el bucle y se desconecta del broker
    client.loop_stop()
    client.disconnect()
