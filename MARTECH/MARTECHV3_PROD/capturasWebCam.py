import cv2
import paho.mqtt.client as mqtt
import base64
import os
import threading
from datetime import datetime, timedelta

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC_CAPTURE = "webcam/capture"
MQTT_TOPIC_PHOTO = "webcam/photo"
MQTT_TOPIC_INFO = "webcam/info"
SAVE_DIR = "D:\MARTECH_WebCam_Capturas"
CAPTURE_INTERVAL_HOURS = 4

os.makedirs(SAVE_DIR, exist_ok=True)

client = mqtt.Client()
client.connect(MQTT_BROKER, MQTT_PORT, 60)

def capture_photo():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        return False
    
    ret, frame = cap.read()
    if ret:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        file_path = os.path.join(SAVE_DIR, f"{timestamp}.jpg")
        cv2.imwrite(file_path, frame)
        client.publish(MQTT_TOPIC_INFO, f"{file_path},{timestamp}")
        with open(file_path, "rb") as image_file:
            jpg_as_text = base64.b64encode(image_file.read()).decode()
            client.publish(MQTT_TOPIC_PHOTO, jpg_as_text)
    
    cap.release()
    return ret

def on_message(client, userdata, msg):
    if msg.topic == MQTT_TOPIC_CAPTURE:
        capture_photo()

def automatic_capture():
    while True:
        capture_photo()
        print(f"Automatic capture at {datetime.now()}")
        threading.Event().wait(timedelta(hours=CAPTURE_INTERVAL_HOURS).total_seconds())

client.on_message = on_message
client.subscribe(MQTT_TOPIC_CAPTURE)

if __name__ == '__main__':
    print("Waiting for capture command...")

    auto_capture_thread = threading.Thread(target=automatic_capture, daemon=True)
    auto_capture_thread.start()

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        client.disconnect()
