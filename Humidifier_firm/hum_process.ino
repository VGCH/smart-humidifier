
void get_DHT22_data(){
      if(millis() - prevMills > 2000) {
          humidity = dht.getHumidity();
          temperature = dht.getTemperature();
          prevMills = millis();
      }
}


void powerS(){
  String response_message ="";
  if(hum_level_low_b){
      digitalWrite(power_control, HIGH);
      delay(4000);
      digitalWrite(power_control, LOW);
      response_message += "<center><a href=\"#\" class=\"btn_off\" onclick=\"on_off()\">Включить увлажнитель</a></center>";
      hum_level_low_b = false;
   }else{
      digitalWrite(power_control, HIGH);
      delay(500);
      digitalWrite(power_control, LOW);
      response_message  += "<center><a href=\"#\" class=\"btn_on\" onclick=\"on_off()\">Выключить увлажнитель</a></center>";
      hum_level_low_b = true;
  }
     server.send(200, "text/html", response_message + "0" +radio_b());
     if(settings.mqtt_en){ MQTT_send_data("json", JSON_DATA());}
}

 String get_mode(int modes){
  String ret = "";
  switch (modes) {
      case 1:
        if(hum_level_low_b && !hum_level_medium_b && !hum_level_max_b){
           ret = "checked";
        }
       break;
     case 2:
      
       if(hum_level_low_b && hum_level_medium_b && !hum_level_max_b){
          ret = "checked";
        }
       break;
     case 3:
      
       if(hum_level_low_b && hum_level_medium_b && hum_level_max_b){
         ret = "checked";
       }
       break;
     default:
       ret = "";
   }
   return ret;
 }

void set_mode_pin(int modes){
    if(hum_level_low_b){
        int ll  = 0;
        int ml  = 0;
        int mxl = 0;
        if(hum_level_low_b){    ll = 1; }else{ ll = 0; };
        if(hum_level_medium_b){ ml = 1; }else{ ml = 0; };
        if(hum_level_max_b){    mxl = 1;}else{ mxl = 0; };
        int mod = ll + ml + mxl;
        int set_m = 0;
        if(mod == 3 && mod != modes){
          set_m = modes;
        }
        if(mod == 2 && mod != modes){
          if(modes == 1){
             set_m = modes + 1;
          }else{
             set_m = modes - 2;
          }
        }
        if(mod == 1 && mod != modes){
          set_m = modes - 1;
        }
      for (int i = 0; i < set_m; i++){
         digitalWrite(power_control, HIGH);
         delay(200); 
         digitalWrite(power_control, LOW);
         delay(200); 
      }
      MQTT_send_data("json", JSON_DATA());
    }
 }


void callback(char* topic, byte* payload, unsigned int length) {
           String message;
         for (int i = 0; i < length; i++) {
             message = message + (char)payload[i];
          }
          if(message != "On" || message != "Off"){
             int data = message.toInt();
             if(data == 0){
                 powerS();
             }else{
                 set_mode_pin(data);
             }
           }
 }

 void pin_pr(){

  if(digitalRead(hum_level_low) && !hum_level_low_b){        hum_level_low_b = true;  }
  if(digitalRead(hum_level_medium) && !hum_level_medium_b){  hum_level_medium_b = true; }
  if(digitalRead(hum_level_max) && !hum_level_max_b){        hum_level_max_b = true; }
  if(digitalRead(water_lvl) && !water_lvl_b){                water_lvl_b = true; }


  if(!digitalRead(hum_level_low) && hum_level_low_b){
    if(!hum_level_low_p){
        low_t = millis();
        hum_level_low_p = true;
      }else{
          if(millis() - low_t > 100){
          hum_level_low_b = false; 
        }
      }
    }
    
  if(!digitalRead(hum_level_medium) && hum_level_medium_b){
    if(!hum_level_medium_p){
       med_t = millis();
       hum_level_medium_p = true;
      }else{
         if(millis() - med_t > 100){
         hum_level_medium_b = false; 
        }
      }
    }
    
  if(!digitalRead(hum_level_max) && hum_level_max_b){
    if(!hum_level_max_p){
      max_t = millis();
      hum_level_max_p = true;
    }else{
       if(millis() - max_t > 100){
          hum_level_max_b = false; 
       }
    }
  }
  
  if(!digitalRead(water_lvl) && water_lvl_b){ 
    if(!water_lvl_p){
        water_t = millis();
        water_lvl_p = true; 
      }else{
         if(millis() - water_t > 2000){
            water_lvl_b = false;
          }
      }
    }

    
  if(digitalRead(hum_level_low)){    hum_level_low_p = false;  }  
  if(digitalRead(hum_level_medium)){ hum_level_medium_p = false;  }
  if(digitalRead(hum_level_max)){    hum_level_max_p = false;  }
  if(digitalRead(water_lvl)){        water_lvl_p = false; }
  
  }
