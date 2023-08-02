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