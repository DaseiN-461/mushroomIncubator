import paho.mqtt.client as mqtt
import datetime
import csv

# Crear un archivo CSV para guardar los mensajes
csv_file = open("leufu.csv", mode="w", newline="")
csv_writer = csv.writer(csv_file)
csv_writer.writerow(["Mensaje"])  # Escribir encabezado

# Define la función que se ejecutará cuando llegue un mensaje MQTT
def on_message(client, userdata, message):
    print(f"Mensaje recibido en el tema '{message.topic}': {message.payload.decode()}")
    msg_str = message.payload.decode("utf-8")
    #print(f"Mensaje recibido en el tópico '{message.topic}': {str(msg_str)}")
    csv_writer.writerow([datetime.datetime.now(),msg_str])
    csv_file.flush()
# Configura el cliente MQTT
client = mqtt.Client()
client.on_message = on_message

# Conéctate al servidor MQTT
broker_address = "mqtt.eclipseprojects.io"  # Cambia esto por la dirección de tu servidor MQTT
port = 1883  # Puerto MQTT predeterminado
client.connect(broker_address, port)

# Suscríbete a un tema (topic) MQTT
topic = "mi_top"  # Cambia esto por el tema al que deseas suscribirte
client.subscribe(topic)

# Inicia el bucle de escucha de mensajes MQTT
client.loop_forever()
