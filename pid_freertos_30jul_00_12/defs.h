////////////////////////////////////////////////// PID Parametros ////////////////////////////////////////////////// 



 typedef struct {
  String id;
  bool enable;
  double setpoint, input, output;
  double kp, ki, kd;
  int windowSize;
  unsigned long windowStartTime;
  unsigned long windowCurrentTime;
} PIDTaskArguments;


PIDTaskArguments pidTempArgs = {
    .id="Temperature",
    .enable=false,
    .setpoint=18,
    .kp=200, .ki=1, .kd=1,
    .windowSize = 2000
  };

PIDTaskArguments pidHumArgs = {
    .id="Humidity",
    .enable=false,
    .setpoint=80,
    .kp=500, .ki=1, .kd=1,
    .windowSize=5000
  };

//////////////////////////////////////// PID /////////////// GPIOs de entradas y salidas

// Relé de salida al actuador de Temperatura
        // PIN de salida
#define RELAY_PIN_HEATER 4
        // Indicador de salida (LED_BUILTIN ESP32 DEVKIT V1)
#define LED_PIN 2

// Relé de salida al actuador de Humedad
#define RELAY_PIN_HUMIDIFIER 12

  
////////////////////////////////////////////////// FreeRTOS //////////////////////////////////////////////////
TaskHandle_t pidTempTaskHandle, pidHumTaskHandle;
TaskHandle_t serialTaskHandle, oledTaskHandle, fsmTaskHandle; 




///////////////////////////////////////////////// DHT22 SENSOR ////////////////////////////////////////////////////

#define DHTPIN 23
#define DHTTYPE DHT21

DHT dht(DHTPIN, DHTTYPE);


////////////////////////////////////////////////// USER INTERFACE BUTTONS //////////////////////////////////////////////////

#define TOUCH_PIN_UP  T9  // Pin táctil T4 pin13
#define TOUCH_PIN_DOWN  T4  // Pin táctil T9 pin32
#define TOUCH_PIN_SEL  T7  // Pin táctil T7 pin27
#define TOUCH_PIN_LEFT  T0  // Pin táctil T0 pin04
#define TOUCH_PIN_RIGHT  T3  // Pin táctil T3 pin15

volatile bool touchUpPressed = false;
volatile bool touchDownPressed = false;
volatile bool touchSelPressed = false;
volatile bool touchLeftPressed = false;
volatile bool touchRightPressed = false;


 

 /////////////////////////////////////////////// FSM estados //////////////////////////////////////////////////////
 enum state_UI{
  
  pid_monitor,

  sel_config,
 
  sel_pid,
  sel_pid_enable,
  sel_pid_setpoint,
  sel_pid_windowSize
  
};

bool pid_temp = true; // pid temp defaul, false = hum
bool config_pid = true;
bool pid_enable = true;



/////////////////////////////////////////// I2C /////////////////////////////////////////////////


#define SDA_PIN 21
#define SCL_PIN 22


///////////////////////////////////////////////// OLED ////////////////////////////////////////////////////

#define OLED_ADDRESS 0X3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// FUNCTIONS ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////// SALIDAS DEL PID ////////////////////////////
// Salida del pin de PID a HIGH
void outputPIDPin_HIGH(String pidID){
        if(pidID == "Temperature"){
                digitalWrite(RELAY_PIN_HEATER, HIGH);
                digitalWrite(LED_PIN, HIGH);
        }else if(pidID == "Humidity"){
                // do output pid humidity HIGH
                digitalWrite(RELAY_PIN_HUMIDIFIER, HIGH);
        }
}

// Salida del pin de PID a LOW
void outputPIDPin_LOW(String pidID){
        if(pidID == "Temperature"){
                digitalWrite(RELAY_PIN_HEATER, LOW);
                digitalWrite(LED_PIN, LOW);
        }else if(pidID == "Humidity"){
                // do output pid humidity LOW
                digitalWrite(RELAY_PIN_HUMIDIFIER, LOW);
        }
}


// 
void pidToActuator(void* pvParameters){
        PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
        
        // Si el tiempo transcurrido desde el inicio de la nueva ventana es mayor que el tamaño de la ventana
        // Entonces reinicia la ventana actualizando el valor de inicio de la nueva ventana
        if (millis() - args->windowStartTime > args->windowSize){ 
                args->windowStartTime += args->windowSize;
        }

        // Actualiza el valor del tiempo transcurrido desde el inicio de la nueva ventana
        args->windowCurrentTime = millis() - args->windowStartTime;

        // Si al salida del control PID es mayor que el tiempo transcurrido desde el inicio de la nueva ventana
        // Entonces activa el pin de salida del relay, sino la desactiva
        if (args->output > millis() - args->windowStartTime){
                outputPIDPin_HIGH(args->id);
        }else{
                outputPIDPin_LOW(args->id);
        }
}


/////////////////////////////////////// TOUCH BUTTONS INTERRUPTIONS ////////////////////////////////

void IRAM_ATTR touchUpISR() {
  touchUpPressed = true;
}

void IRAM_ATTR touchDownISR() {

  touchDownPressed = true;
}

void IRAM_ATTR touchSelISR() {
  touchSelPressed = true;
}
void IRAM_ATTR touchLeftISR() {
  touchLeftPressed = true;
}
void IRAM_ATTR touchRightISR() {
  touchRightPressed = true;
 }
