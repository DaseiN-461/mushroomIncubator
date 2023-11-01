#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

//const char* ssid = "MiSSID";
//const char* password = "MiContraseña";
#include <PubSubClient.h>
const char* mqttServer = "mqtt.eclipseprojects.io";
const int mqttPort = 1883;
const char* mqttTopic = "mi_topico";

WiFiClient espClient;
PubSubClient client(espClient);


char storedSSID[32] = "";
char storedPassword[32] = "";


const int ssidAddress = 0;
const int passwordAddress = 32;

bool accessPointMode = false;
const unsigned long accessPointTimeout = 10000;  // Tiempo de espera para el modo de punto de acceso (10 segundos)

AsyncWebServer server(80);

float setpoint = 0.0;
float t_i = 1.1;
float t_o = 2.2;
float h_i = 3.0;
float h_o = 4.3;

// Tu mensaje a publicar
char message[50];

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



String crearMensaje(float valor1, float valor2, float valor3) {
  // Calcular el tamaño necesario para la cadena
  String mensaje = String(valor1) + "," + String(valor2) + "," + String(valor3);
  
  return mensaje;
}


void loop() {
  
}

bool connectToWiFi() {
  WiFi.begin(storedSSID, storedPassword);
  unsigned long startAttemptTime = millis();

  Serial.println("Intentando la conexión a la red");
  Serial.println("SSID: "+ String(storedSSID));

  Serial.print("Conectando a la red Wi-Fi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    
    if (millis() - startAttemptTime >= accessPointTimeout) {
      Serial.println("No se pudo conectar a la red. Cambiando al modo de punto de acceso.");
      return false;
    }
  }
  
  Serial.print("\nConexión exitosa a la red Wi-Fi.\n\t ---------------> SSID: " + String(storedSSID) + ", localIP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void EEPROM_setup(){
  EEPROM.begin(512);
  EEPROM.get(ssidAddress, storedSSID);
  EEPROM.get(passwordAddress, storedPassword);
}

void startWebServer() {
  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<h1>Hola, mundo</h1>";
    request->send(200, "text/html", html);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<h1>Configuración de Credenciales de WiFi</h1>";
    html += "<form method='post' action='/config'>";
    html += "SSID: <input type='text' name='ssid' value=''><br><br>";
    html += "Contraseña: <input type='password' name='password' value=''><br><br>";
    html += "<input type='submit' value='Guardar Credenciales'>";
    html += "</form>";
    request->send(200, "text/html", html);
  });

  // Manejar la solicitud POST para configurar las credenciales
  server.on("/config", HTTP_POST, handleConfig);

  server.begin();

  // Obtiene y muestra la dirección IP
  Serial.print("Servidor web iniciado. IP: ");
  Serial.println(WiFi.localIP());
}

void startAccessPoint() {
  accessPointMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ConfigurarWiFi");
  Serial.println("Modo de punto de acceso iniciado. Conéctate a la red 'ConfigurarWiFi' e ingresa las nuevas credenciales en la página web.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void handleConfig(AsyncWebServerRequest *request) {
  String newSSID = request->arg("ssid");
  String newPassword = request->arg("password");

  if (newSSID.length() > 0 && newPassword.length() > 0) {
    newSSID.toCharArray(storedSSID, 32);
    newPassword.toCharArray(storedPassword, 32);
    EEPROM.put(ssidAddress, storedSSID);
    EEPROM.put(passwordAddress, storedPassword);
    EEPROM.commit();
    Serial.println("Credenciales guardadas en la EEPROM.");
    
    if (accessPointMode) {
      Serial.println("Reiniciando en modo estación.");
      delay(2000);
      ESP.restart();
    } else {
      request->send(200, "text/html", "Credenciales guardadas. Reiniciando para conectar a la nueva red Wi-Fi.");
      delay(2000);
      ESP.restart();
    }
  } else {
    request->send(200, "text/html", "Error: Ingresa valores válidos para SSID y contraseña.");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error al conectar, estado: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}