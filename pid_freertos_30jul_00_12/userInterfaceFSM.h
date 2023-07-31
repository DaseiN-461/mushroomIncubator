






Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Finite State Machine /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

state_UI currentStateUI = pid_monitor;




void state_selConfig_printOled();
void state_selPID_printOled();
void state_configSetpoint_printOled(double setpoint);
void state_pidEnable_printOled();

void stateMachine(state_UI currentState, void* pvParameters){
  PIDTaskArguments* args = (PIDTaskArguments*)pvParameters;
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
                   
        }
        


        touchRightPressed = false;  
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
          touchSelPressed = false;
          
          currentStateUI = sel_pid_enable;
       
          Serial.println("pid is selected, are you enable?");
          
        }       
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


        if(touchRightPressed&&pid_enable){
          touchRightPressed = false;
          
          currentStateUI = sel_pid_setpoint;
          Serial.println("pid is enable, whats setpoint?");
            
          
        }
        
        break;
        
      case sel_pid_setpoint:
       
          // PID Temperatura: AJUSTE DE VALOR DE SETPOINT 
          
          if(pid_enable && pid_temp){ 
                  //boton de UP
                  if(touchUpPressed){
                          touchUpPressed = false;
      
                          args->setpoint++;
                          
                          Serial.printf("sp: %f\n",args->setpoint);
                          
                  }
                  //boton de DOWN
                  if (touchDownPressed) {
                          touchDownPressed = false;
                          
                          args->setpoint--;
                          
                          Serial.printf("sp: %f\n",args->setpoint);
                  }
                  
          // PID Humedad: AJUSTE DE VALOR DE SETPOINT 
           
          }else if(pid_enable && !pid_temp){
                  if(touchUpPressed){
                          touchDownPressed = false;

                          //pidHumArgs.setpoint++;
                          Serial.println("subiendole a la humedad");  
                  }
                  if(touchDownPressed){
                          touchDownPressed = false;

                          //pidHumArgs.setpoint--;
                          Serial.println("bajandole a la humedad");  
                          
                  }
          }


        if(touchLeftPressed){
          touchLeftPressed = false;
          

          Serial.println("setpoint is saved, going to monitor ->");
          currentStateUI= pid_monitor;
          
        }
        state_configSetpoint_printOled(args->setpoint);
        
        break;  
  }
  
 }

 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Finite State Machine /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
              oled.println("select PID\n\n\n");
              if(pid_temp){
                
                oled.print("[PID temperatura]\n");
              }else{
                oled.print("[PID humedad]\n");
              }
              oled.display();
}

void state_pidEnable_printOled(){
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
}

void state_configSetpoint_printOled(double setpoint){
                      oled.clearDisplay();
                      oled.setCursor(0,0);
                      oled.printf("setpoint: %f",setpoint);
                      
                      oled.display();
}
