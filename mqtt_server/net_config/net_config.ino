#include <Arduino.h>
#include "mqtt.h"





float setpoint = 0.0;
float t_i = 1.1;
float t_o = 2.2;
float h_i = 3.0;
float h_o = 4.3;



void setup() {
  Serial.begin(115200);
  EEPROM_setup();

  WiFi.mode(WIFI_STA);

  if (!connectToWiFi()) {       // Si no ha conectado a la red registrada
    startAccessPoint();                     // Crea un punto de acceso para configurar las credenciales de la red
    startWebServer();                       // Crea un servidor web asincrono http para realizar la configuración
  }else{                      //   Si se ha conectado a la red registrada
    //Optional
    //startWebServer();       // Podría llevar otra web para otro tipo de configuracion com

    client.setServer(mqttServer, mqttPort); // Configura el servidor mqtt 

    while(true){
      // Tu bucle principal
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

  




      
      
      String data = crearMensaje(t_i, t_o, h_i);
      
      char* message = strdup(data.c_str());


      if (client.publish(mqttTopic, message)) {
        Serial.println("Mensaje publicado en MQTT");
      } else {
        Serial.println("Error al publicar el mensaje en MQTT");
      }
      free(message);
      delay(5000); // Intervalo de publicación
    }

  }

  // Resto del codigo
  
  
}

void loop() {
  
}

