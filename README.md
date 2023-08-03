# mushroomIncubator
The following repository supports the development of an environmental conditioning system intended for mushroom cultivation. The system monitors and controls temperature and relative humidity. The main program is developed for an ESP32 dual-core microcontroller using FreeRTOS APIs to create and manage task execution.

> Aunque el propósito del desarrollo de este sistema es el cultivo de hongos, este puede ser utilizado para otros fines que requiera el condicionamiento de ambiente. 
>> Contact us [Instagram](https://www.instagram.com/aonde_la_byte/).

# Features
 * Un pid de temperatura y uno de humedad relativa.
 * Cuenta con una interfaz de usuario que permite configurar y monitorear el funcionamiento de los controles PID.
 * La configuración establecida se guarda automáticamente. Al reiniciar el sistema, inicia en la configuración establecida.
 * Comunicación serial de los datos para su registro desde una computadora.

# Futuras mejoras
 * Registro de datos en una memoria micro SD.
 * Interfaz web.

# Definición de pines
A continuación se presenta la definición de los pines y una breve descripción de cada uno.
| Identificador  | GPIO | Descripción |
| ------------- |:-------------:|:-------------:|
| HEATER      |D34| Salida digital al actuador de temperatura.|
| LED BUILTIN |D2| Indicador LED interno de la placa de desarrollo, está conectado a la salida D34.|
| HUMIDIFIER     |D12| Salida digital al actuador de humedad.|
|DHT 21|D23| Entrada digital del sensor de temperatura y humedad.
|TOUCH UP|D13| Entrada táctil para el envío de comando arriba.
|TOUCH DOWN|D32|Entrada táctil para el envío de comando abajo.
|TOUCH SELECT|D27|Entrada táctil para el envío de comando selección.
|TOUCH BACK|D4| Entrada táctil para el envío de comando atrás.
|TOUCH NEXT|D15|Entrada táctil para el envío de comando siguiente.
|SDA|D21| Intercambio de datos por comunicación i2c con la pantalla.
|SCL|D22| Señal de sincronización de la comunicación i2c con la pantalla.