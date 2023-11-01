import paho.mqtt.client as mqtt
import csv

# Crear un archivo CSV para guardar los mensajes
csv_file = open("mensajes.csv", mode="w", newline="")
csv_writer = csv.writer(csv_file)
csv_writer.writerow(["Mensaje"])  # Escribir encabezado



# Configuración del broker y el tópico
broker_address = "mqtt.eclipseprojects.io"
port = 1883
topic = "mi_topico"    # Reemplaza con el tópico que deseas

# Callback cuando se recibe un mensaje
def on_message(client, userdata, message):
    msg_str = message.payload.decode("utf-8")
    print(f"Mensaje recibido en el tópico '{message.topic}': {str(msg_str)}")
    csv_writer.writerow([msg_str])
    csv_file.flush()
# Crear un cliente MQTT
client = mqtt.Client("suscriptor")

# Configurar la función de callback para recibir mensajes
client.on_message = on_message

# Conexión al broker
client.connect(broker_address, port)

# Suscribirse al tópico
client.subscribe(topic)

# Mantener el programa en ejecución para recibir mensajes
client.loop_forever()
