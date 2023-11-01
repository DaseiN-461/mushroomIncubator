import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk
import csv
import paho.mqtt.client as mqtt
import threading

# Función para iniciar la comunicación serial en un hilo separado
def iniciar_comunicacion():
    global comunicacion_activa
    puerto_serial = puerto_combobox.get()
    baud_rate = int(baudrate_entry.get())
    
    try:
        ser = serial.Serial(port=puerto_serial, baudrate=baud_rate, timeout=1)
        estado_label.config(text=f"Comunicación Serial Iniciada en {puerto_serial}")
        
        # Bucle para recibir y guardar mensajes en tiempo real
        while comunicacion_activa:
            mensaje = ser.readline().decode("utf-8")
            csv_writer.writerow([mensaje])
            csv_file.flush()  # Asegurar que los datos se escriben en el archivo
            print(mensaje)  # Opcional: mostrar el mensaje en la consola
            publicar_mensaje_mqtt(mensaje)  # Publicar el mensaje MQTT
    except Exception as e:
        estado_label.config(text=f"Error al iniciar la comunicación: {str(e)}")

# Función para publicar el mensaje a través de MQTT
def publicar_mensaje_mqtt(mensaje):
    topic = "mi_topico"  # Tópico MQTT
    client = mqtt.Client("publicador_mqtt")
    
    try:
        client.connect("mqtt.eclipseprojects.io", 1883)
        client.publish(topic, mensaje)
        client.disconnect()
    except Exception as e:
        print(f"Error al publicar el mensaje MQTT: {str(e)}")

# Función para cerrar la comunicación serial y el programa
def cerrar_programa():
    global comunicacion_activa
    comunicacion_activa = False
    csv_file.close()  # Cerrar el archivo CSV
    ventana.destroy()  # Cerrar la ventana

# Crear una ventana
ventana = tk.Tk()
ventana.title("Comunicación Serial y MQTT")
ventana.geometry("300x250")

# Crear un archivo CSV para guardar los mensajes
csv_file = open("mensajes.csv", mode="w", newline="")
csv_writer = csv.writer(csv_file)
csv_writer.writerow(["Mensaje"])  # Escribir encabezado

# Etiqueta de selección de puerto
puerto_label = tk.Label(ventana, text="Seleccionar Puerto Serial:")
puerto_label.pack(pady=10)

# ComboBox para listar los puertos seriales disponibles
puertos_disponibles = serial.tools.list_ports.comports()
puertos = [port.device for port in puertos_disponibles]
puerto_combobox = ttk.Combobox(ventana, values=puertos)
puerto_combobox.pack()

# Etiqueta y entrada para la velocidad de baudios
baudrate_label = tk.Label(ventana, text="Baud Rate:")
baudrate_label.pack()
baudrate_entry = tk.Entry(ventana)
baudrate_entry.insert(0, "115200")  # Valor predeterminado
baudrate_entry.pack()

# Botón para iniciar la comunicación serial
iniciar_button = tk.Button(ventana, text="Iniciar Comunicación", command=lambda: threading.Thread(target=iniciar_comunicacion).start())
iniciar_button.pack(pady=10)

# Botón para cerrar el programa
cerrar_button = tk.Button(ventana, text="Cerrar Programa", command=cerrar_programa)
cerrar_button.pack()

# Etiqueta de estado
estado_label = tk.Label(ventana, text="")
estado_label.pack()

# Variable global para controlar la comunicación serial
comunicacion_activa = True

# Cierre de la ventana
ventana.protocol("WM_DELETE_WINDOW", cerrar_programa)

ventana.mainloop()
