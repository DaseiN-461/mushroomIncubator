#include "libs.h"
#include "defs.h"
#include "userInterfaceFSM.h"




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
      
      
        xTaskCreatePinnedToCore(task_MONITOR, "[oled] monitor task", 10*1024, NULL, 1, &oledTaskHandle,1);
        xTaskCreatePinnedToCore(task_FSM, "[oled] user interface fsm", 20*1024, &pidTempArgs, 2, &fsmTaskHandle,1);
        
        xTaskCreatePinnedToCore(mainTask, "main Task", 10*1024, NULL, 5, NULL,1);
        
      
       
      
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
          
                // Mientras no se obtenga una lectura del sensor adecuada, el sistema no continúa y se queda detenida en el búcle
                while(!measureOK){
                        if(args->id=="Temperature"){
                                args->input = dht.readTemperature();
                        }else if(args->id=="Humidity"){
                                args->input = dht.readHumidity();
                        }
                        if (isnan(args->input)) {
                                Serial.println(F("Failed to read from DHT sensor!"));
                                // Advertencia en pantalla OLED sobre la falla en el sensor.
                        }else{
                                measureOK = true;
                        }
                }

                // Actualiza el valor de salida del PID
                pid.Compute();

                // La salida del PID la utiliza para modular las conmutaciones del rele de salida
                pidToActuator(pvParameters);
            
                measureOK = false;
                vTaskDelay(50/portTICK_PERIOD_MS);
        }
}







void task_MONITOR(void* pvParameters){
  
        while(1){
                oled.clearDisplay();
                oled.setCursor(0,0);
            
                oled.printf("[PID temp] - sp:%.2f\n\tin:%.2f|out:%.2f",pidTempArgs.setpoint,pidTempArgs.input,pidTempArgs.output);
                oled.printf("\n\t time:%lu|en:%i \n -------------------",pidTempArgs.windowCurrentTime, pidTempArgs.enable);
                oled.printf("\n[PID hum] - sp:%.2f\n\tin:%.2f|out:%.2f",pidHumArgs.setpoint,pidHumArgs.input,pidHumArgs.output);
                oled.printf("\n\t time:%lu",pidHumArgs.windowCurrentTime);
                
                oled.display();
                
                vTaskDelay(200/portTICK_PERIOD_MS);
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

                        // Si al menos un botón está siendo presionado.
                        //Si la tarea estaba suspendida, se reanuda.
                        if(eTaskGetState(fsmTaskHandle)==eSuspended){
                                vTaskResume(fsmTaskHandle);
                          
                        }
                }else{
                        // Si ningún botón es presionado.
                        // Si la tarea no estaba suspendida, se suspende.
                        if(eTaskGetState(fsmTaskHandle)!= eSuspended){
                                vTaskSuspend(fsmTaskHandle);
                        }
                }

                // Condición para administrar la tarea de PID:

                ///////////////////////// PID TEMPERATURA ////////////////////////////////////
                 
                // Si el valor de enable está activo.
                if(pidTempArgs.enable){
                        // Si la tarea estaba suspendida
                        if(eTaskGetState(pidTempTaskHandle)==eSuspended){
                                // La reanuda
                                Serial.println("pidtemp reanudado");
                                vTaskResume(pidTempTaskHandle);
                          
                        }
                // Si esta inactivo
                }else{
                        // Si no estaba suspendida
                        if(eTaskGetState(pidTempTaskHandle)!= eSuspended){
                                // La suspende
                                Serial.println("pidtemp suspendido");
                                vTaskSuspend(pidTempTaskHandle);
                        }
                }

                ///////////////////////// PID HUMEDAD ////////////////////////////////////
                
                if(pidHumArgs.enable){
                        // Si la tarea estaba suspendida
                        if(eTaskGetState(pidHumTaskHandle)==eSuspended){
                                // La reanuda
                                vTaskResume(pidHumTaskHandle);
                          
                        }
                // Si esta inactivo
                }else{
                        // Si no estaba suspendida
                        if(eTaskGetState(pidHumTaskHandle)!= eSuspended){
                                // La suspende
                                vTaskSuspend(pidHumTaskHandle);
                        }
                }


                // Condición para administrar la tarea de TX serial: 
                vTaskDelay(100/portTICK_PERIOD_MS);
        }
}

void task_FSM(void* pvParameters){
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
                 vTaskDelay(50/portTICK_PERIOD_MS);        
        } 
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
