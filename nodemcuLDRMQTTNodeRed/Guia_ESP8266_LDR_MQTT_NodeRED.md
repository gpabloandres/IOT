# Guía para poner en funcionamiento el circuito de medición de luz con ESP8266 + LDR + MQTT + Node-RED

## Paso 1: Conectar los dispositivos a la misma red
- Asegurarse de que **la PC** y el **ESP8266** estén conectados a la misma red WiFi.  
- Apuntar el nombre de la red (SSID) y la contraseña.  

## Paso 2: Verificar direcciones IP
- Abrir la terminal de Windows (CMD) y ejecutar:  
  ```
  ipconfig
  ```  
- Identificar la **IP de la PC**.  
- La **IP del ESP8266** se verá luego en el **monitor serial** del IDE de Arduino una vez cargado el programa.  

## Paso 3: Conectar y programar el ESP8266
- Abrir el **IDE de Arduino**.  
- Seleccionar la **placa ESP8266** (Ejemplo: NodeMCU 1.0).  
- Seleccionar el **puerto COM** correcto.  
- Cargar un programa que:  
  - Configure el **WiFi** con el SSID y contraseña.  
  - Lea el valor del **sensor LDR** conectado a un pin analógico (A0).  
  - Se conecte al **servidor MQTT** (Broker).  
  - Publique los datos de la medición en un **tópico MQTT** (ejemplo: `casa/habitacion/luz`).  

## Paso 4: Verificar conexión
- Abrir el **monitor serial** del IDE de Arduino.  
- Revisar que:  
  - El ESP8266 se conecte correctamente a la red WiFi.  
  - Se muestre la **IP asignada al ESP8266**.  
  - Se confirme la conexión al broker MQTT.  

## Paso 5: Configurar el broker MQTT
- Si se usa **Mosquitto** en la PC como broker:  
  - Asegurarse de que esté instalado y en ejecución.  
  - Usar el puerto estándar **1883**.  
- Si hay problemas de conexión por IP:  
  - Revisar con:  
    ```
    netsh interface portproxy show all
    ```  
  - En caso de conflicto, borrar y volver a crear la regla con:  
    ```
    netsh interface portproxy delete v4tov4 listenaddress=192.168.x.x listenport=1883
    netsh interface portproxy add v4tov4 listenaddress=MI_IP_LOCAL listenport=1883 connectaddress=127.0.0.1 connectport=1883
    ```  

## Paso 6: Configurar Node-RED
- Abrir Node-RED en el navegador (`http://localhost:1880`).  
- Añadir un **nodo MQTT IN** y configurarlo con:  
  - Servidor: la IP de la PC (o `localhost` si Node-RED y Mosquitto están en la misma máquina).  
  - Puerto: 1883.  
  - Tópico: el mismo que usa el ESP8266 (`casa/habitacion/luz`).  
- Conectar el nodo a un **debug** para probar que llegan los datos.  

## Paso 7: Crear la interfaz web
- Usar los nodos de **Node-RED Dashboard** (`node-red-dashboard`).  
- Insertar un **gráfico o gauge** para mostrar en tiempo real los valores de luz recibidos.  
- Probar la interfaz desde:  
  ```
  http://localhost:1880/ui
  ```  

---
✅ Con esto, el circuito estará funcionando: el **ESP8266 mide la luz con el LDR**, **envía los datos por MQTT**, y **Node-RED los recibe y muestra en la web**.  
