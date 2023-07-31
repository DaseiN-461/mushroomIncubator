////////////////////////////////////////////////// PID Parametros ////////////////////////////////////////////////// 


#define RELAY_PIN 4

#define LED_PIN 2

 typedef struct {
  String id;
  double setpoint, input, output;
  double kp, ki, kd;
  int windowSize;
  unsigned long windowStartTime;
  unsigned long windowCurrentTime;
} PIDTaskArguments;


///////////////////////////////////////////////// DHT22 SENSOR ////////////////////////////////////////////////////

#define DHTPIN 23
#define DHTTYPE DHT21

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
