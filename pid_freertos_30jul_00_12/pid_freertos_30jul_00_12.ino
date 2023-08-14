#include "libs.h"
#include "defs.h"
#include "userInterfaceFSM.h"




void setup()
{
  
        Serial.begin(115200);
        Wire.begin(SDA_PIN, SCL_PIN);

        
        connectSensor();

        // La memoria EEPROM del ESP32 es de 4096 BYTES
        EEPROM.begin(4096);

        // Cargar la configuracion guardada en la EEPROM
        load_config();
      
      
      
        pinMode(LED_PIN, OUTPUT);
        pinMode(RELAY_PIN_HEATER,OUTPUT);
        pinMode(RELAY_PIN_HUMIDIFIER,OUTPUT);
       
      
        // Configurar los pines táctiles como entradas
        touchAttachInterrupt(TOUCH_PIN_UP, touchUpISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
        touchAttachInterrupt(TOUCH_PIN_DOWN, touchDownISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
        touchAttachInterrupt(TOUCH_PIN_SEL, touchSelISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
        touchAttachInterrupt(TOUCH_PIN_BACK, touchBackISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
        touchAttachInterrupt(TOUCH_PIN_NEXT, touchNextISR, 20); // Umbral de sensibilidad táctil (ajustar según necesidades)
      
        
        oledSetup();

      
        xTaskCreatePinnedToCore(task_PID, "PID temp", 20*1024, &pidTempArgs, 3, &pidTempTaskHandle,0);
        xTaskCreatePinnedToCore(task_PID, "PID hum", 20*1024, &pidHumArgs, 3, &pidHumTaskHandle,0);
        
        xTaskCreatePinnedToCore(task_serialPrint, "serial TX", 5*1024, NULL, 1, &serialTaskHandle,1);
       
      
        xTaskCreatePinnedToCore(task_MONITOR, "[oled] monitor task", 10*1024, NULL, 1, &oledTaskHandle,1);
        xTaskCreatePinnedToCore(task_FSM, "[oled] user interface fsm", 20*1024, &pidTempArgs, 5, &fsmTaskHandle,1);
        
        xTaskCreatePinnedToCore(mainTask, "main Task", 10*1024, NULL, 2, NULL,1);
        
      
       
      
        //vTaskSuspend(pidTempTaskHandle);
        //vTaskSuspend(pidHumTaskHandle);
        //vTaskSuspend(serialTaskHandle);
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
        
        while(1){
          
                if(args->id=="Temperature"){
                        args->input = temperatureMeasure();
                }else if(args->id=="Humidity"){
                        args->input = humidityMeasure();
                }

                // Actualiza el valor de salida del PID
                pid.Compute();

                // La salida del PID la utiliza para modular las conmutaciones del rele de salida
                pidToActuator(pvParameters);
            
                isConnected = false;
                vTaskDelay(500/portTICK_PERIOD_MS);
        }
}







void task_MONITOR(void* pvParameters){
  
        while(1){
                
                state_monitor_printOled();
                //16ms for 60<frames/second
                vTaskDelay(200/portTICK_PERIOD_MS);
        }
}





// Tarea para administrar las tareas: Monitor y máquina de estados
void mainTask(void* pvParameters){

        while(1){
                //////////////////////////////////////////////////////////////////////////////
                ///////////////////////////// MONITOR TASK //////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////
                // Condicion para administrar la tarea del monitor: cuando no se está utilizando.
                if(currentStateUI == pid_monitor){
                        // Si el estado actual de la máquina es el monitor y la tarea se encuentra suspendida,
                        // entonces reanuda la tarea del monitor.
                        if(eTaskGetState(oledTaskHandle)==eSuspended){
                                vTaskResume(oledTaskHandle);
                        }
                }else{
                        // Si el estado actual no es el monitor y la tarea no se encuentra suspendida,
                        // entonces suspende la tarea del monitor.
                        if(eTaskGetState(oledTaskHandle)!= eSuspended){
                                vTaskSuspend(oledTaskHandle);
                        }      
                }

                //////////////////////////////////////////////////////////////////////////////
                ///////////////////////////// USER INTERFACE FSM TASK //////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////
                // Condicion para administrar la tarea de la máquina de estados: si ningún botón es presionado.
                if(touchUpPressed or touchDownPressed or touchBackPressed or touchNextPressed or touchSelPressed){

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

                //////////////////////////////////////////////////////////////////////////////
                ///////////////////////////// MONITOR TASK //////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////
                // Condición para administrar la tarea de PID:

                ///////////////////////// PID TEMPERATURA ////////////////////////////////////
                 
                // Si el valor de enable está activo.
                if(pidTempArgs.enable){
                        // Si la tarea estaba suspendida
                        if(eTaskGetState(pidTempTaskHandle)==eSuspended){
                                // La reanuda
                                vTaskResume(pidTempTaskHandle);
                          
                        }
                // Si esta inactivo
                }else{
                        // Si no estaba suspendida
                        if(eTaskGetState(pidTempTaskHandle)!= eSuspended){
                                // La suspende
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

                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                ///////////////////////////////////////// SERIAL TX ////////////////////////////////////////////////////////////////
                // Condición para administrar la tarea de TX serial: 
                if(pidTempArgs.enable or pidHumArgs.enable){
                        
                }
                

                
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
                
                if(pidTempArgs.enable or pidHumArgs.enable){
                        if(pidTempArgs.enable){
                                Serial.printf("%.2f,%.2f",pidTempArgs.input,pidTempArgs.output);
                                if(pidHumArgs.enable){
                                          Serial.print(",");
                                }
                        }
                        if(pidHumArgs.enable){
                                Serial.printf("%.2f,%.2f",pidHumArgs.input,pidHumArgs.output);
                        }
                        Serial.println();
                }
                
                
                vTaskDelay(1000/portTICK_PERIOD_MS);
        }
}
