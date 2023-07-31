






Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Finite State Machine /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

state_UI currentStateUI = pid_monitor;



void oledSetup();
void state_monitor_printOled();
void state_selConfig_printOled();
void state_selPID_printOled();
void state_configSetpoint_printOled(double setpoint);
void state_pidEnable_printOled();

void stateMachine(state_UI currentState, void* pvParameters){
        PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
        switch(currentStateUI){

                // Si el estado es monitor de los PID (Estado natural del sistema)
                case pid_monitor:

                        // Si el boton de Seleccion es pulsado
                        if (touchSelPressed){
                                // El estado siguiente es la seleccion del tipo de configuracion
                                currentStateUI = sel_config;
                                touchSelPressed = false;
                        }
                        break;

                // Si el estado es la seleccion del tipo de configuracion
                case sel_config:
                  
                        // Si el boton Arriba es presionado
                        if(touchUpPressed){
                                // No se seleccionó la configuración de PID 
                                // Entonces el tipo de configuración es de TX SERIAL
                                config_pid = false;
                                touchUpPressed = false;        
                        }
                        
                        // Si el boton Abajo es presionado
                        if(touchDownPressed){
                                // Se seleccionó la configuración PID
                                config_pid = true;
                                touchDownPressed = false;
                        }
                  
                        // Si el boton de Derecha es presionado
                        if(touchRightPressed){
                                // Si se seleccionó la configuración PID
                                if(config_pid){
                                        // Entonces el siguiente estado es la seleccion del PID
                                        currentStateUI = sel_pid;            
                                }
                          touchRightPressed = false;  
                        }
                  
                        // Si el boton Izquierda es presionado
                        if (touchLeftPressed){
                                // Entonces vuelve al estado MONITOR
                                currentStateUI = pid_monitor;
                                touchLeftPressed = false;
                        }
                        break;
                        
                // Si el estado es la seleccion del PID
                case sel_pid:
                        // Si el boton Arriba es presionado
                        if(touchUpPressed){
                                //  Se selecciona el PID de temperatura
                                pid_temp = true;
                                touchUpPressed = false;
                        }
                        // Si el boton Abajo es presionado
                        if(touchDownPressed){
                                // Se selecciono el PID de humedad
                                pid_temp = false;
                                touchDownPressed = false;
                        }
                        // Si el boton Select es presionado
                        if(touchSelPressed){
                                // Se confirma la selección y el siguiente estado es la activación del PID
                                currentStateUI = sel_pid_enable;
                                touchSelPressed = false;
                        }   
                        if(touchLeftPressed){
                                // Vuelve a la selección de configuración
                                currentStateUI = sel_config;
                        }
                        break;
                  
                  // Si el estado es la seleccion de activacion del PID
                  case sel_pid_enable:
            
                          if(touchUpPressed){
                                  pid_enable = true;
                                  touchUpPressed = false;
                          }
                  
                          if(touchDownPressed){
                                  pid_enable = false;
                                  touchDownPressed = false;
                            
                          }

                          // Si el boton Derecha es presionado, se confirma la seleccion
                          if(touchRightPressed){
                                  // Si el PID seleccionado es el de temperatura
                                  if(pid_temp){
                                          // Si se habilito
                                          if(pid_enable){
                                                  // Ajustar el valor de la estructura
                                                  pidTempArgs.enable=true;
                                          // Si se deshabilito
                                          }else{
                                                  pidTempArgs.enable=false;
                                          }
                                          
                                  // Si el PID seleccionado es el de humedad
                                  }else{
                                          // Si se habilito
                                          if(pid_enable){
                                                  // Ajustar el valor de la estructura
                                                  pidHumArgs.enable=true;
                                          // Si se deshabilito
                                          }else{
                                                  pidHumArgs.enable=false;
                                          }
                                 
                                  }
                                  // El siguiente estado es la selección del SETPOINT
                                  currentStateUI = sel_pid_setpoint;
                                  touchRightPressed = false;                              
                            
                          }
                          
                          break;

                  // Estado de selección del SETPOINT del PID escogido
                  case sel_pid_setpoint:
                          // PID Temperatura: AJUSTE DE VALOR DE SETPOINT 
                          
                          if(pid_enable && pid_temp){ 
                                  // Si el botón de UP es presionado
                                  if(touchUpPressed){
                                          pidTempArgs.setpoint++;
                                          touchUpPressed = false;                                                      
                                  }
                                  //boton de DOWN
                                  if (touchDownPressed) {
                                          pidTempArgs.setpoint--;
                                          touchDownPressed = false;
                                  }
                                  
                                  // Se imprime en la pantalla OLED el SETPOINT seleccionado
                                  state_configSetpoint_printOled(pidTempArgs.setpoint);
                                  
                          // PID Humedad: AJUSTE DE VALOR DE SETPOINT 
                           
                          }else if(pid_enable && !pid_temp){
                                  if(touchUpPressed){
                                          pidHumArgs.setpoint++;
                                          touchDownPressed = false; 
                                  }
                                  if(touchDownPressed){
                                          pidHumArgs.setpoint--;
                                          touchDownPressed = false;                                      
                                  }
                                  // Se imprime en la pantalla OLED el SETPOINT seleccionado
                                  state_configSetpoint_printOled(pidHumArgs.setpoint);
                          }
                        
    
                        // Si el boton Izquierda es presionado, se confirma la seleccion
                        if(touchLeftPressed){
                                // El siguiente estado es el monitor de los PID
                                currentStateUI= pid_monitor;
                                touchLeftPressed = false;                          
                        }
                        
                        break;  
        }
        touchLeftPressed = false;
        touchRightPressed = false;
        touchUpPressed = false;
        touchDownPressed = false;
        touchSelPressed = false;
  
 }

 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Finite State Machine /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void state_monitor_printOled(){
                oled.clearDisplay();
                oled.setCursor(0,0);
            
                if(pidTempArgs.enable or pidHumArgs.enable){
                        if(pidTempArgs.enable){
                                oled.printf("[PID temp] - sp:%.2f\n\tin:%.2f|out:%.2f",pidTempArgs.setpoint,pidTempArgs.input,pidTempArgs.output);
                                oled.printf("\n\t time:%lu|on/off:%i \n -------------------",pidTempArgs.windowCurrentTime,pidTempArgs.enable);
                        }
                        if(pidHumArgs.enable){
                                oled.printf("\n[PID hum] - sp:%.2f\n\tin:%.2f|out:%.2f",pidHumArgs.setpoint,pidHumArgs.input,pidHumArgs.output);
                                oled.printf("\n\t time:%lu|on/off:%i",pidHumArgs.windowCurrentTime,pidHumArgs.enable);   
                        }
                }else{
                        oled.println("my\n\tlittle\n\t\tmushRoom\n\nSEL:CONFIG");
                }
                oled.display();
}

void state_selConfig_printOled(){
              oled.clearDisplay();
              oled.setCursor(0,0);
              oled.print("select configuration\n\n");
              oled.print("UP: TX UART (!)\nDOWN: PID\nLEFT: BACK TO MONITOR\nRIGHT: GO NEXT");
              oled.print("\n\n\t->[");
              if(config_pid){
                oled.print("PID");
              }else{
                oled.print("NO DISPONIBLE");
              }
              oled.print("]");
              oled.display();
}

void state_selPID_printOled(){
              oled.clearDisplay();
              oled.setCursor(0,0);
              oled.println("select PID\n");
              if(pid_temp){
                
                oled.print("->[PID temperatura]\n");
              }else{
                oled.print("->[PID humedad]\n");
              }
              oled.print("UP: TEMPERATURA\nDOWN:HUMEDAD\nSEL:SELECT\nLEFT: GO BACK\nRIGHT:");
              oled.display();
}

void state_pidEnable_printOled(){
              oled.clearDisplay();
              oled.setCursor(0,0);
              if(pid_temp){
                oled.print("[PID temperatura]\n");
              }else{
                oled.print("[PID humedad]\n");
              }
              if(pid_enable){
                oled.print("->ON\n");
              }else{
                oled.print("->OFF\n");
              }

              oled.print("UP:ON\nDOWN:OFF\nRIGHT: GO NEXT\nLEFT: UNDEFINED\nSEL: UNDEFINED");
              
              oled.display();
}

void state_configSetpoint_printOled(double setpoint){
                      oled.clearDisplay();
                      oled.setCursor(0,0);
                      oled.printf("setpoint: %f\n",setpoint);

                      oled.print("UP:   +\n");
                      oled.print("DOWN: -\n");
                      oled.print("LEFT: SET AND SAVE\n");
                      oled.print("RIGHT: UNDEFINED\n");
                      oled.print("SEL: UNDEFINED\n");
                      oled.display();
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
  oled.println("hello mushlover");
  oled.display();


  
}
