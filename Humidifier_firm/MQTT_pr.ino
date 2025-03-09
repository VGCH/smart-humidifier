// Функция отправки данных по протоколу MQTT
void MQTT_send(){
   client.loop();
   uint32_t nows = millis();
  if (nows - lastMsg > 2000 && settings.mqtt_en) {
    if(WiFi.status() == WL_CONNECTED) {
        MQTT_send_data("jsondata", JSON_DATA());
     }
   lastMsg = nows; 
  }
 }
void MQTT_send_data(String topic, String data){
         String root = settings.mqtt_topic;
         String top  = root +"/"+ topic;
         String subscr = root +"/control";
         String input = settings.mqtt_serv;
         int colonIndex = input.indexOf(':');
         String ipAddress;
         String port;
         IPAddress Remote_MQTT_IP;

        if (colonIndex != -1) {
             ipAddress = input.substring(0, colonIndex);
             port = input.substring(colonIndex + 1);
             WiFi.hostByName(ipAddress.c_str(), Remote_MQTT_IP);
          }
       
     // client.setServer(ipAddress.c_str(), port.toInt());
      client.setServer(Remote_MQTT_IP, port.toInt());
      client.setCallback(callback);
      client.setBufferSize(1024); 
           if(client.connected()){
                 count_rf = 0; 
                 send_mqtt(top, data, subscr);
           }else{
              count_rf++;
              if (client.connect(settings.mqtt_id.c_str(), settings.mqtt_user.c_str(), settings.mqtt_passw.c_str())){           
                    send_mqtt(top, data, subscr);
                }else{
                  if(count_rf > 2){
                     WiFi.disconnect();
                     WiFi.begin(settings.mySSID, settings.myPW);
                     count_rf = 0;
                }
            }
        }
     
}

void send_mqtt(String tops, String data, String subscr){
     // Анонсируем датчики и переключатели для Home Assistant [auto-discovery ]
     // Анонсируем элементы один раз при успешном подуключении и при запуске устройства
    //if(!annonce_mqtt_discovery){
          // Уникальный идентификатор устройства
          String device_id = "humidifier_" + ch_id;
          // Имя устройства
          String device_name = "Cyberex Smart Humidifier";
          // Модель устройства (опционально)
          String device_model = "SCM v1.0";
          // Производитель устройства (опционально)
          String device_manufacturer = "CYBEREX TECH";
          // Версия ПО
          String sw = "1.1";

          // JSON-конфигурация устройства
          String device_config = "\"device\": {\"ids\": [\"" + device_id + "\"], \"name\": \"" + device_name + "\", \"mdl\": \"" + device_model + "\", \"sw\": \"" + sw + "\", \"mf\": \"" + device_manufacturer + "\"}";

          String top      = String(settings.mqtt_topic) +"/jsondata";
          String control  = String(settings.mqtt_topic) +"/control";
          //Анонс датчика температуры
          String ditance = "{\"device_class\": \"temperature\", \"name\": \"Датчик температуры\", \"state_topic\": \""+top+"\", \"unit_of_measurement\": \"°C\", \"value_template\": \"{{ value_json.temp}}\", \"unique_id\": \"" + ch_id + "_temp\", " + device_config + " }";
          String conf1 = "homeassistant/sensor/"+ch_id+"_temp/config";
          client.publish(conf1.c_str(), ditance.c_str(), true);
          //Анонс датика влажности 
          String conf2 = "homeassistant/sensor/"+ch_id+"_hum/config";
          String ditance_up = "{\"device_class\": \"humidity\", \"name\": \"Датчик влажности\", \"state_topic\": \""+top+"\", \"unit_of_measurement\": \"%\", \"value_template\": \"{{ value_json.hum }}\", \"unique_id\": \"" + ch_id + "_hum\", " + device_config + " }";
          client.publish(conf2.c_str(), ditance_up.c_str(), true);
          //Анонс датчика уровня
          String conf3 = "homeassistant/binary_sensor/"+ch_id+"_level/config";
          String ditance_down = "{\"device_class\": \"moisture\", \"name\": \"Уровень воды\", \"state_topic\": \""+top+"\", \"payload_on\": \"0\", \"payload_off\": \"1\", \"value_template\": \"{{ value_json.water }}\", \"unique_id\": \"" + ch_id + "_level\", " + device_config + " }";
          client.publish(conf3.c_str(), ditance_down.c_str(), true);
          //Анонс переключателя On/Off  
          String conf4 = "homeassistant/switch/"+ch_id+"_on_off/config";
          String Playload_on_off = "{\"name\": \"Питание увлажнителя\", \"command_topic\": \""+control+"\", \"state_topic\": \""+top+"\", \"payload_on\": \"0\", \"payload_off\": \"0\", \"state_on\": \"On\", \"state_off\": \"Off\", \"value_template\": \"{{ value_json.chanel1 }}\", \"unique_id\": \"" + ch_id + "_on_off\", " + device_config + "}";
          client.publish(conf4.c_str(), Playload_on_off.c_str(), true);
          //Анонс переключателя Режим мин
          String conf5 = "homeassistant/switch/"+ch_id+"min/config";
          String Playload_auto = "{\"name\": \"Режим минимальный\", \"command_topic\": \""+control+"\", \"state_topic\": \""+top+"\", \"payload_on\": \"1\", \"state_on\": \"On\", \"state_off\": \"Off\", \"value_template\": \"{% if value_json.chanel1 == \'On\' and value_json.level2 == \'0\' and value_json.level3 == \'0\' %} On {% else %} Off {% endif %}\", \"unique_id\": \"" + ch_id + "min\", " + device_config + "}";
          client.publish(conf5.c_str(), Playload_auto.c_str(), true);

          //Анонс переключателя Режим средний
          String conf6 = "homeassistant/switch/"+ch_id+"mid/config";
          String Playload_mid = "{\"name\": \"Режим средний\", \"command_topic\": \""+control+"\", \"state_topic\": \""+top+"\", \"payload_on\": \"2\", \"state_on\": \"1\", \"state_off\": \"0\", \"value_template\": \"{% if value_json.chanel1 == \'On\' and value_json.level2 == \'1\' and value_json.level3 == \'0\' %} 1 {% else %} 0 {% endif %}\", \"unique_id\": \"" + ch_id + "mid\", " + device_config + "}";
          client.publish(conf6.c_str(), Playload_mid.c_str(), true);

          //Анонс переключателя Режим макс
          String conf7 = "homeassistant/switch/"+ch_id+"max/config";
          String Playload_max = "{\"name\": \"Режим максимальный\", \"command_topic\": \""+control+"\", \"state_topic\": \""+top+"\", \"payload_on\": \"3\", \"state_on\": \"1\", \"state_off\": \"0\", \"value_template\": \"{% if value_json.chanel1 == \'On\' and value_json.level2 == \'1\' and value_json.level3 == \'1\' %} 1 {% else %} 0 {% endif %}\", \"unique_id\": \"" + ch_id + "max\", " + device_config + "}";
          client.publish(conf7.c_str(), Playload_max.c_str(), true);
          //annonce_mqtt_discovery = true;
   // }
    // Отправляем данные 
    client.publish(tops.c_str(), data.c_str());
    client.subscribe(subscr.c_str());
}

String MQTT_status(){
    String response_message = "";
    if(client.connected()){
         response_message = "подключен";
      }else{
         response_message = "отключен";
    }
    return response_message;
} 
