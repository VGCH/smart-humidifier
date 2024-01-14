 String millis2time(){ // преобразование миллисекунд в вид д/ч/м/с
  
         int times =(millis()/1000);
         int days =  (times/3600)/24;
         int timehour =(((times)  % 86400L) / 3600); // часы
        if ( ((times % 3600) / 60) < 10 ) {
         int timehour = 0;
               }
         int timeminuts=((times  % 3600) / 60); // минуты
         if ( (times % 60) < 10 ) {
         int timeminuts = 0;
             }
         int timeseconds=(times % 60); // секунды
       String Time= String(days)+":"+String(twoDigits(timehour))+":"+String(twoDigits(timeminuts))+":"+String(twoDigits(timeseconds));
       return Time;
     }

 String twoDigits(int digits){
             if(digits < 10) {
          String i = '0'+String(digits);
          return i;
         }
          else {
        return String(digits);
         }
      }

void time_work(){
   if (captivePortal()) {  
    return;
  }
  String header;
  if (validateToken()){
   String outjson  ="{";
          outjson += "\"time\":\""+millis2time()+"\",";
          outjson += "\"chanel1\":";
  if(hum_level_low_b){
          outjson  += "\"On\",";
     }else{
          outjson  += "\"Off\",";
           }
          outjson += "\"temp\":";
          outjson += "\""+String(temperature)+"\",";
          outjson += "\"hum\":";
          outjson += "\""+String(humidity)+"\",";
          outjson += "\"level1\":";
          outjson += "\""+String(hum_level_low_b)+"\",";
          outjson += "\"level2\":";
          outjson += "\""+String(hum_level_medium_b)+"\",";
          outjson += "\"level3\":";
          outjson += "\""+String(hum_level_max_b)+"\",";
          outjson += "\"water\":";
          outjson += "\""+water_l()+"\",";
          outjson += "\"MQTT\":\""+MQTT_status()+"\"";
          outjson += "}";
     server.send(200, "text", outjson);   
  }   
}

String water_l(){
  String lw = "В норме";
  if(water_lvl_b){
    lw = "Низкий";
  }
  return lw;
}

String JSON_DATA(){
     String outjson  = "{";
            outjson += "\"chanel1\":";
     if(hum_level_low_b){
            outjson  += "\"On\",";
        }else{
            outjson  += "\"Off\",";
           }
          outjson += "\"temp\":";
          outjson += "\""+String(temperature)+"\",";
          outjson += "\"hum\":";
          outjson += "\""+String(humidity)+"\",";
          outjson += "\"level1\":";
          outjson += "\""+String(hum_level_low_b)+"\",";
          outjson += "\"level2\":";
          outjson += "\""+String(hum_level_medium_b)+"\",";
          outjson += "\"level3\":";
          outjson += "\""+String(hum_level_max_b)+"\",";
          outjson += "\"water\":";
          outjson += "\""+String(water_lvl_b)+"\"";
          outjson += "}";  
     return outjson;
}
