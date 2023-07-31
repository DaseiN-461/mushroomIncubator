#include "libs.h"
#include "defs.h"
#include "userInterfaceFSM.h"


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

////////////////////////////////////////////////// FreeRTOS //////////////////////////////////////////////////

TaskHandle_t pidTempTaskHandle, pidHumTaskHandle;
TaskHandle_t serialTaskHandle, oledTaskHandle, fsmTaskHandle; 



DHT dht(DHTPIN, DHTTYPE);

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


void oledSetup();

void setup()
{
  
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();



  pinMode(LED_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
 

  // Configurar los pines táctiles como entradas
  touchAttachInterrupt(TOUCH_PIN_UP, touchUpISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_DOWN, touchDownISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_SEL, touchSelISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_LEFT, touchLeftISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
  touchAttachInterrupt(TOUCH_PIN_RIGHT, touchRightISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)

  
  oledSetup();
  

  xTaskCreatePinnedToCore(task_PID, "PID temp", 20*1024, &pidTempArgs, 3, &pidTempTaskHandle,0);
  xTaskCreatePinnedToCore(task_PID, "PID hum", 20*1024, &pidHumArgs, 3, &pidHumTaskHandle,0);
  
  //xTaskCreatePinnedToCore(task_serialPrint, "serial print temp", 1*1024, &pidTempArgs, 1, &serialTaskHandle,1);
  //xTaskCreatePinnedToCore(task_serialPrint, "serial print hum", 1*1024, &pidHumArgs, 1, &serialTaskHandle,1);


  xTaskCreatePinnedToCore(oledTask, "oled task", 10*1024, NULL, 1, &oledTaskHandle,1);
  
  xTaskCreatePinnedToCore(taskUserInterfaceFSM, "user interface fsm", 10*1024, &pidTempArgs, 1, &fsmTaskHandle,1);
  xTaskCreatePinnedToCore(mainTask, "main Task", 10*1024, NULL, 2, NULL,1);
  

 

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
    
    vTaskDelay(50/portTICK_PERIOD_MS);
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


// Tarea para administrar las tareas: Monitor y máquina de estados
void mainTask(void* pvParameters){

        while(1){
          
                // Condicion para administrar la tarea del monitor: cuando no se está utilizando.
                if(currentStateUI == pid_monitor){
                        // Si el estado actual de la máquina es el monitor y la tarea se encuentra suspendida,
                        // entonces reanuda la tarea del monitor.
                        if(eTaskGetState(oledTaskHandle)==eSuspended){
                                vTaskResume(oledTaskHandle);
                                Serial.println("hello from pid_monitor");
                        }
                }else{
                        // Si el estado actual no es el monitor y la tarea no se encuentra suspendida,
                        // entonces suspende la tarea del monitor.
                        if(eTaskGetState(oledTaskHandle)!= eSuspended){
                                Serial.println("fsmTaskSuspended");
                                vTaskSuspend(oledTaskHandle);
                        }      
                }
            
                // Condicion para administrar la tarea de la máquina de estados: si ningún botón es presionado.
                if(touchUpPressed or touchDownPressed or touchLeftPressed or touchRightPressed or touchSelPressed){
                        Serial.println("un boton fue presionado");
                        
                        // Si al menos un botón está siendo presionado.
                        //Si la tarea estaba suspendida, se reanuda.
                        if(eTaskGetState(fsmTaskHandle)==eSuspended){
                                vTaskResume(fsmTaskHandle);
                          
                        }
                  
                        
                }else{
                        // Si ningún botón es presionado.
                        // Si la tarea no estaba suspendida, se suspende.
                        if(eTaskGetState(fsmTaskHandle)!= eSuspended){
                                Serial.println("fsmTaskSuspended");
                                vTaskSuspend(fsmTaskHandle);
                        }
                }
                vTaskDelay(100/portTICK_PERIOD_MS);
        }
}

void taskUserInterfaceFSM(void* pvParameters){
  PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
  while(1){
   
    

    stateMachine(currentStateUI, args);
    
    
    
    switch(currentStateUI){
      
            case sel_config:
              state_selConfig_printOled();
              break;
              
            case sel_pid:
              state_selPID_printOled();
              break;
            case sel_pid_enable:
              state_pidEnable_printOled();
              break;

              
            
          }
          
          vTaskDelay(200/portTICK_PERIOD_MS);    
  } 
}
