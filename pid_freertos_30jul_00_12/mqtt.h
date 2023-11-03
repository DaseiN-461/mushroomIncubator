#include <EEPROM.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>



#include <PubSubClient.h>
const char* mqttServer = "mqtt.eclipseprojects.io";
const int mqttPort = 1883;
const char* mqttTopic = "mi_top";


WiFiClient espClient;
PubSubClient client(espClient);


char storedSSID[32] = "";
char storedPassword[32] = "";


const int ssidAddress = 100;
const int passwordAddress = 132;

bool accessPointMode = false;
const unsigned long accessPointTimeout = 10000;  // Tiempo de espera para el modo de punto de acceso (10 segundos)

AsyncWebServer server(80);

// Tu mensaje a publicar
char message[50];

String crearMensaje(float valor1, float valor2, float valor3,float valor4, float valor5, float valor6) {
  // Calcular el tamaño necesario para la cadena
  String mensaje = String(valor1) + "," + String(valor2) + "," + String(valor3) + "," + String(valor4) + "," + String(valor5) + "," + String(valor6);
  
  return mensaje;
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

  EEPROM.get(ssidAddress, storedSSID);
  EEPROM.get(passwordAddress, storedPassword);
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