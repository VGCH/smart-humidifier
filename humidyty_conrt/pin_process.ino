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
