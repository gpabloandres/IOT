# Guía para controlar un LED con ESP8266 + MQTT + Node-RED mediante un botón en la interfaz web

## Paso 1: Conectar los dispositivos a la misma red
- Asegurarse de que **la PC** y el **ESP8266** estén conectados a la misma red WiFi.  
- Apuntar el nombre de la red (SSID) y la contraseña.  

## Paso 2: Conectar el LED al ESP8266
- Conectar el LED con una resistencia de 220Ω en serie:  
  - Ánodo (positivo) → Pin digital del ESP8266 (ej. D2).  
  - Cátodo (negativo) → GND.  

## Paso 3: Programar el ESP8266 en Arduino IDE
- Abrir el **IDE de Arduino**.  
- Seleccionar la **placa ESP8266** (Ejemplo: NodeMCU 1.0).  
- Seleccionar el **puerto COM** correcto.  
- Instalar la librería `PubSubClient` para trabajar con MQTT.  
- Cargar un programa que:  
  - Configure la conexión WiFi con el SSID y contraseña.  
  - Se conecte al **broker MQTT** (Mosquitto).  
  - Se suscriba a un tópico MQTT (ejemplo: `casa/habitacion/led`).  
  - Encienda o apague el LED según el mensaje recibido (`ON` o `OFF`).  

## Paso 4: Verificar conexión
- Abrir el **monitor serial** del IDE de Arduino.  
- Confirmar que:  
  - El ESP8266 se conecte correctamente a la red WiFi.  
  - Se muestre la **IP asignada al ESP8266**.  
  - Se confirme la conexión al broker MQTT.  
  - Se indique que está suscrito al tópico `casa/habitacion/led`.  

## Paso 5: Configurar el broker MQTT
- Si se usa **Mosquitto** en la PC como broker:  
  - Asegurarse de que esté instalado y en ejecución.  
  - Usar el puerto estándar **1883**.  

## Paso 6: Configurar Node-RED
- Abrir Node-RED en el navegador (`http://localhost:1880`).  
- Añadir un **nodo botón (ui_button)** desde `node-red-dashboard`.  
- Configurar el botón para que:  
  - Envíe el mensaje `"ON"` al presionarlo.  
  - Envíe el mensaje `"OFF"` al soltarlo o con un segundo botón.  
- Conectar el botón a un nodo **MQTT OUT** configurado con:  
  - Servidor: la IP de la PC (o `localhost` si Node-RED y Mosquitto están en la misma máquina).  
  - Puerto: 1883.  
  - Tópico: `casa/habitacion/led`.  

## Paso 7: Probar la interfaz web
- Abrir el panel web de Node-RED en:  
  ```
  http://localhost:1880/ui
  ```  
- Pulsar el botón y comprobar que el LED conectado al ESP8266 enciende y apaga según la orden.  

---
✅ Con esto, el circuito estará funcionando: el **botón en Node-RED envía un mensaje MQTT**, el **ESP8266 lo recibe** y **controla el LED** en tiempo real.  
