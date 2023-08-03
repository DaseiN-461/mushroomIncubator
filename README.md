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
 * Configuración de la frecuencia de medición, transmisión de datos y registro.
 * Configuración de tamaño de la ventana y cotas para el mínimo y máximo de la salida PWM.
 * Añadir restricciones para el ingreso de parámetros 
 * Interfaz web.

# Descrición del sistema electrónico

Se realizó el programa destinado a un microcontrolador ESP32. Este se encarga de obtener las mediciones de temperatura y humedad a través de un sensor DHT 21, compara la diferencia entre el valor medido y el deseado (error), calcula y genera una señal de potencia promedio proporcional al error que modula las conmutaciones de los relés a los actuadores. Además, el microcontrolador despliega información en una pantalla OLED recibe interrupciones mediante cinco botones táctiles que permiten navegar a través de una interfaz de usuario.

# Definición de pines
A continuación se presenta la definición de los pines y una breve descripción de cada uno.
| Identificador  | GPIO | Descripción |
| ------------- |:-------------:|:-------------:|
| HEATER      |D16| Salida digital al actuador de temperatura.|
| LED BUILTIN |D2| Indicador LED interno de la placa de desarrollo, está conectado a la salida D34.|
| HUMIDIFIER     |D17| Salida digital al actuador de humedad.|
|DHT 21|D34| Entrada digital del sensor de temperatura y humedad.
|TOUCH UP|D32| Entrada táctil para el envío de comando arriba.
|TOUCH DOWN|D33|Entrada táctil para el envío de comando abajo.
|TOUCH SELECT|D27|Entrada táctil para el envío de comando selección.
|TOUCH BACK|D14| Entrada táctil para el envío de comando atrás.
|TOUCH NEXT|D12|Entrada táctil para el envío de comando siguiente.
|SDA|D21| Intercambio de datos por comunicación i2c con la pantalla.
|SCL|D22| Señal de sincronización de la comunicación i2c con la pantalla.

