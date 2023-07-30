

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

////////////////////////////////////////////////// FreeRTOS //////////////////////////////////////////////////

TaskHandle_t pidTempTaskHandle, pidHumTaskHandle;
TaskHandle_t serialTaskHandle, oledTaskHandle; 


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
////////////////////////////////////////////////// FSM Definicion de estados //////////////////////////////////////////////////


enum state_UI{
  
  pid_monitor,

  sel_config,
 
  sel_pid,
  sel_pid_enable,
  sel_pid_setpoint,
  sel_pid_windowSize
  
};
state_UI currentStateUI = sel_config;

bool pid_temp = true; // pid temp defaul, false = hum
bool config_pid = false;
bool pid_enable = false;
int count = 0;
void stateMachine(state_UI currentState);



////////////////////////////////////////////////// PID Parametros ////////////////////////////////////////////////// 
#include <PID_v1.h>

#define RELAY_PIN 4

#define LED_PIN 2


////////////////////////////////////////////////// DHT 22 SENSOR //////////////////////////////////////////////////

#include "DHT.h"
#define DHTPIN 23
#define DHTTYPE DHT21

DHT dht(DHTPIN, DHTTYPE);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



typedef struct {
  String id;
  double setpoint, input, output;
  double kp, ki, kd;
  int windowSize;
  unsigned long windowStartTime;
  unsigned long windowCurrentTime;
} PIDTaskArguments;


////////////////////////////////////////////////// BUS I2C /////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#define SDA_PIN 21
#define SCL_PIN 22

////////////////////////////////////////////////// Librerías de la OLED SSD1306 ////////////////////////////////////////////////////
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_ADDRESS 0X3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PIDTaskArguments pidTempArgs = {
    .id="Temperature",
    .setpoint=32,
    .kp=10, .ki=1, .kd=1,
    .windowSize = 2000
  };

PIDTaskArguments pidHumArgs = {
    .id="Humidity",
    .setpoint=85,
    .kp=10, .ki=1, .kd=1,
    .windowSize=5000
  };

typedef struct{
  PIDTaskArguments pidTemp;
  PIDTaskArguments pidHum;
} OLEDTaskArguments;



void oledSetup();

void setup()
{
  
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();



  pinMode(LED_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
 

  // Configurar los pines táctiles como entradas
  touchAttachInterrupt(TOUCH_PIN_UP, touchUpISR, 12); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_DOWN, touchDownISR, 12); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_SEL, touchSelISR, 12); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_LEFT, touchLeftISR, 12); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_RIGHT, touchRightISR, 12); // Umbral de sensibilidad táctil (ajustar según necesidades)

  
  oledSetup();
  

  xTaskCreatePinnedToCore(task_PID, "PID temp", 20*1024, &pidTempArgs, 3, &pidTempTaskHandle,0);
  xTaskCreatePinnedToCore(task_PID, "PID hum", 20*1024, &pidHumArgs, 3, &pidHumTaskHandle,0);
  
  //xTaskCreatePinnedToCore(task_serialPrint, "serial print temp", 1*1024, &pidTempArgs, 1, &serialTaskHandle,1);
  //xTaskCreatePinnedToCore(task_serialPrint, "serial print hum", 1*1024, &pidHumArgs, 1, &serialTaskHandle,1);


  xTaskCreatePinnedToCore(oledTask, "oled task", 5*1024, NULL, 1, &oledTaskHandle,1);
  
  xTaskCreatePinnedToCore(taskUserInterfaceFSM, "user interface fsm", 10*1024, NULL, 2, NULL,1);
  

 

  //vTaskSuspend(pidTempTaskHandle);
  //vTaskSuspend(pidHumTaskHandle);
  vTaskSuspend(serialTaskHandle);
  //vTaskSuspend(oledTaskHandle);
  
}

void loop()
{
  

}

void task_PID(void* pvParameters){
  PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
  PID pid(&args->input, &args->output, &args->setpoint, args->kp, args->ki, args->kd, DIRECT);
  
  //tell the PID to range between 0 and the full window size
  pid.SetOutputLimits(0, args->windowSize);
  //turn the PID on
  pid.SetMode(AUTOMATIC);

  

  args->windowStartTime = millis();
  bool measureOK = false;
  while(1){
    

    while(!measureOK){
      if(args->id=="Temperature"){
        args->input = dht.readTemperature();
      }else if(args->id=="Humidity"){
        args->input = dht.readHumidity();
      }
      if (isnan(args->input)) {
        Serial.println(F("Failed to read from DHT sensor!"));
       
      }else{
        measureOK = true;
      }
    }
  
    pid.Compute();

    
    pidToActuator(pvParameters);

    measureOK = false;
    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}

void outputPIDPin_HIGH(String pidID){
  if(pidID == "Temperature"){
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  }else if(pidID == "Humidity"){
    // do output pid humidity HIGH
  }
}
void outputPIDPin_LOW(String pidID){
  if(pidID == "Temperature"){
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }else if(pidID == "Humidity"){
    // do output pid humidity LOW
  }
}

void pidToActuator(void* pvParameters){
  PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
  
  if (millis() - args->windowStartTime > args->windowSize)
    { //time to shift the Relay Window
      args->windowStartTime += args->windowSize;
    }
    args->windowCurrentTime = millis() - args->windowStartTime;
    if (args->output > millis() - args->windowStartTime){
      outputPIDPin_HIGH(args->id);
    }else{
      outputPIDPin_LOW(args->id);
    }
}








void oledTask(void* pvParameters){
  
  while(1){
    oled.clearDisplay();
    oled.setCursor(0,0);

    oled.printf("[PID temp] - sp:%.2f\n\tin:%.2f|out:%.2f",pidTempArgs.setpoint,pidTempArgs.input,pidTempArgs.output);
    oled.printf("\n\t time:%lu \n -------------------",pidTempArgs.windowCurrentTime);
    oled.printf("\n[PID hum] - sp:%.2f\n\tin:%.2f|out:%.2f",pidHumArgs.setpoint,pidHumArgs.input,pidHumArgs.output);
    oled.printf("\n\t time:%lu",pidHumArgs.windowCurrentTime);
    
    oled.display();
    
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void oledSetup(){
  if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  oled.display();
  delay(2000);

  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0,0);
  oled.println("hello-world");
  oled.display();


  
}

void task_serialPrint(void* pvParameters){
  PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
  while(1){
      Serial.printf("\n----------- PID %s", args->id);
      Serial.printf("\t input: %f, output %f\n",args->input, args->output);

      //16ms for 60<frames/second
      vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void taskUserInterfaceFSM(void* pvParameters){
  while(1){
    
    
    stateMachine(currentStateUI);

    
    
    
    if(currentStateUI == pid_monitor){
      if(eTaskGetState(oledTaskHandle)==eSuspended){
        vTaskResume(oledTaskHandle);
      }
      
    }else{
      //if(eTaskGetState(oledTaskHandle)!=eSuspended){
        vTaskSuspend(oledTaskHandle);
      //}
      
    }
    
    switch(currentStateUI){
      case sel_pid_setpoint:
        oled.clearDisplay();
        oled.setCursor(0,0);
        oled.printf("setpoint: %f",pidTempArgs.setpoint-1);
        
        oled.display();
        break;
      case sel_config:
        oled.clearDisplay();
        oled.setCursor(0,0);
        if(config_pid){
          oled.print("PID\n");
        }else{
          oled.print("MODO\n");
        }
        oled.display();
        break;
        
      case sel_pid:
        oled.clearDisplay();
        oled.setCursor(0,0);
        oled.println("Seleccione el PID");
        if(pid_temp){
          
          oled.print("[PID temperatura]\n");
        }else{
          oled.print("[PID humedad]\n");
        }
        oled.display();
        break;
      case sel_pid_enable:
        oled.clearDisplay();
        oled.setCursor(0,0);
        if(pid_temp){
          oled.print("[PID temperatura]\n\t");
        }else{
          oled.print("[PID humedad]\n\t");
        }
        if(pid_enable){
          oled.print("ON");
        }else{
          oled.print("OFF");
        }
        oled.display();
        break;
      
    }
    

    
    vTaskDelay(50/portTICK_PERIOD_MS);
  } 
}

void stateMachine(state_UI currentState){
  switch(currentStateUI){
    
    case pid_monitor:
      
      if (touchSelPressed){
        currentStateUI = sel_config;
        
        Serial.println("config");
  
        touchSelPressed = false;
      }
      break;

    case sel_config:
      
      //select config
      if(touchUpPressed){
        config_pid = false;
        Serial.println("mode");
        touchUpPressed = false;        
      }
      if(touchDownPressed){
        config_pid = true;
        Serial.println("pid");
        touchDownPressed = false;
      }

      // go to the config selected
      if(touchRightPressed){
        if(config_pid){
          Serial.println("select pid");
          currentStateUI = sel_pid;
        
       
          touchRightPressed = false;
        }
      }

      // back to the monitor
      if (touchLeftPressed){
        currentStateUI = pid_monitor;
        Serial.println("monitor");
        touchLeftPressed = false;
      }
      break;


    case sel_pid:
        
        if(touchUpPressed){
          touchUpPressed = false;
          pid_temp = true;
          Serial.println("temp");
        }
        if(touchDownPressed){
          touchDownPressed = false;
          pid_temp = false;
          Serial.println("hum");
        }
        if(touchSelPressed){
          currentStateUI = sel_pid_enable;
          Serial.println("pid is selected, are you enable?");
          touchSelPressed = false;
        }

/*
        // back to the sel config
        if (touchLeftPressed){
          currentStateUI = sel_config;
          Serial.println("config");
          touchLeftPressed = false;
          delay(1000);
        }
*/
   /*
        // back to the monitor
        if (touchSelPressed){
          
          currentStateUI = pid_monitor;
          Serial.println("monitor");
          delay(1000);
          touchSelPressed = false;
        }
    */  
        
        break;
      
      case sel_pid_enable:

        if(touchUpPressed){
          pid_enable = true;
          Serial.println("enable");
          touchUpPressed = false;
        }

        if(touchDownPressed){
          pid_enable = false;
          Serial.println("disable");
          touchDownPressed = false;
        }


        if(pid_enable){
          
          if(touchRightPressed){
            currentStateUI = sel_pid_setpoint;
            Serial.println("pid is enable, whats setpoint?");
            touchRightPressed = false;
          }
        }
        
        break;
        
      case sel_pid_setpoint:
        //boton de U
        if (touchUpPressed) {
          touchUpPressed = false;

          if(pid_enable && pid_temp){
            pidTempArgs.setpoint++;
            Serial.printf("sp: %f\n",pidTempArgs.setpoint);
          
          }else if(pid_enable && !pid_temp){
            Serial.println("subiendole a la humedad");
          }
          
        }
      
        //boton de D
        if (touchDownPressed) {
          touchDownPressed = false;
          pidTempArgs.setpoint--;
          Serial.printf("sp: %f\n",pidTempArgs.setpoint-1);
          
        }


        if(touchLeftPressed){
          touchLeftPressed = false;

          Serial.println("setpoint is saved, going to monitor ->");
          currentStateUI= pid_monitor;
          
        }
        
        break;  
  }
 }
