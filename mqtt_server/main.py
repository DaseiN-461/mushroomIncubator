import paho.mqtt.client as mqtt
import keyboard

# Configuración del broker y el tema
broker_address = "mqtt.eclipseprojects.io"
port = 1883
topic = "mi_topico"  



# Crear un cliente MQTT
client = mqtt.Client("publicador")

# Conexión al broker
client.connect(broker_address, port)

# Publicar un mensaje en el tópico
message = "Hola desde el publicador MQTT"



# Cierre de la conexión



print("Presiona 'x' para cerrar el programa.")

while True:
        try:
                if keyboard.is_pressed('t'):
                        print("Enviando mensaje.")
                        client.publish(topic, message)
                if keyboard.is_pressed('x'):
                        print("Cerrando programa.")
                        client.disconnect()
                        break
        except:
                break

