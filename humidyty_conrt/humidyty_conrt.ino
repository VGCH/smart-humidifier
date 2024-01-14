#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266SSDP.h>
#include <WiFiUdp.h>
#include "DHTesp.h"
#include <Wire.h>
#include <ESP_EEPROM.h>
#include <PubSubClient.h>
const char* host2 = "smartdevice";
const char *ap_ssid = "SmartHumContr";
const char *ap_password = "EsPnEtWoRk";
const char* mqtt_server = "192.168.1.208";
const char *mqtt_user = "esp8266"; // Логин от серверa
const char *mqtt_pass = "19820830"; // Пароль от сервера
const char *mqtt_client_id = "DF-03-9F-CA-4E-00"; // id устройства
#define power_control 5
#define hum_level_low 12 //3
#define hum_level_medium 14 //4
#define hum_level_max 16 //5
#define water_lvl 13 //2
#define DHT22_pin 4
String callbackq = "No data";
float humidity = 0.0;
float temperature = 0.0;

boolean hum_level_low_b, hum_level_medium_b, hum_level_max_b, water_lvl_b;
boolean hum_level_low_p, hum_level_medium_p, hum_level_max_p, water_lvl_p;
//timers
uint32_t prevMills = 0;
uint32_t lastMsg = 0;
uint32_t low_t, med_t, max_t, water_t;
//
DHTesp dht;
IPAddress apIP(192, 168, 4, 1); 
IPAddress subnet(255,255,255,0); 

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
  EEPROM.begin(2048);
  httpUpdater.setup(&server);
  MDNS.begin(host2);
  MDNS.addService("http", "tcp", 80); 
  delay(10);
   dht.setup(DHT22_pin, DHTesp::DHT22);
   pinMode(power_control, OUTPUT);
   pinMode(hum_level_low, INPUT);
   pinMode(hum_level_medium, INPUT);
   pinMode(hum_level_max, INPUT);
   pinMode(water_lvl, INPUT);
   
   server.on("/",   rootPageHandler);
   server.on("/times.html", testpage);
   server.on("/wlan_config", wlanPageHandler);
   server.on("/json.html", json);
   server.on("/hum.html", hum);
   server.on("/temp.html", temp);
   server.on("/lew.html", lew_w);
   server.on("/powerS", powerS);
   server.on("/mqtt_status", MQTT_status);
   server.on("/mqtt_call", MQTT_callback);
   server.on("/set_mode.html", set_mode);
   server.onNotFound(handleNotFound);
   WiFi.hostname("IoT Smart Hum Controller");
   WiFi.mode(WIFI_STA);
   String ssid = read_EEPROM(2);
   String passw = read_EEPROM(42);
   if(ssid.length() > 2){
      WiFi.begin(ssid.c_str(), passw.c_str());
   }
    for (int x = 0; x < 200; x ++){
    if (WiFi.status() == WL_CONNECTED){ 
      break;
       }
      delay(200); 
       }
 
  if(WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(ap_ssid, ap_password); // Wifi.softAP(ssid, password)
        }
  delay(2000);
  HTTP_init();
  SSDP_init();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
 server.handleClient();
 get_DHT22_data();
 MQTT_send();
 pin_pr();
}

void rootPageHandler() 
{
  String response_message = "<html>";
  response_message +="<head>";
  response_message +="<title>Интерфейс IoT Контроллера увлажнителя</title>";
  response_message += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8;\" />";
  response_message += "<style type=\"text/css\">body{background-color: #e1ff00;color:#FFF;}a {color:#e1ff00;}.blockk {border:solid 1px #2d2d2d;text-align:center;background:#008b8b;padding:10px 10px 10px 10px;-moz-border-radius: 0px;-webkit-border-radius: 0px;border-radius: 0px;}";
  response_message += ".blockk{border:double 2px #000000;-moz-border-radius: 0px;-webkit-border-radius: 0px;border-radius: 0px;}";
  response_message +="</style><style type=\"text/css\" media=\'(min-width: 810px)\'>body{font-size:18px;}.blockk {width: auto;}</style>";
 // response_message +="<style type=\"text/css\" media=\'(max-width: auto) and (orientation:landscape)\'>body{font-size:8px;}</style>
  response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req = new XMLHttpRequest();\n";
  response_message +="req.open(\"GET\",\"times.html\",true);\n";
  response_message +="req.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"xz\").innerHTML = req.responseText;\n";
  response_message +=" }\n";
  response_message +=" req.send();\n";
  response_message +="}\n";
  response_message +="</script>";

  response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req1 = new XMLHttpRequest();\n";
  response_message +="req1.open(\"GET\",\"temp.html\",true);\n";
  response_message +="req1.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"xz1\").innerHTML = req1.responseText;\n";
  response_message +=" }\n";
  response_message +=" req1.send();\n";
  response_message +="}\n";
  response_message +="</script>";

  response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req2 = new XMLHttpRequest();\n";
  response_message +="req2.open(\"GET\",\"hum.html\",true);\n";
  response_message +="req2.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"xz2\").innerHTML = req2.responseText;\n";
  response_message +=" }\n";
  response_message +=" req2.send();\n";
  response_message +="}\n";
  response_message +="</script>";

  response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req3 = new XMLHttpRequest();\n";
  response_message +="req3.open(\"GET\",\"lew.html\",true);\n";
  response_message +="req3.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"xz3\").innerHTML = req3.responseText;\n";
  response_message +=" }\n";
  response_message +=" req3.send();\n";
  response_message +="}\n";
  response_message +="</script>";

  response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req4 = new XMLHttpRequest();\n";
  response_message +="req4.open(\"GET\",\"mqtt_status\",true);\n";
  response_message +="req4.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"mqtt\").innerHTML = req4.responseText;\n";
  response_message +=" }\n";
  response_message +=" req4.send();\n";
  response_message +="}\n";
  response_message +="</script>";

    response_message +="<script>\n";
  response_message +="setInterval(server_time,1000);\n";
  response_message +="function server_time(){\n";
  response_message +="var req5 = new XMLHttpRequest();\n";
  response_message +="req5.open(\"GET\",\"mqtt_call\",true);\n";
  response_message +="req5.onreadystatechange = function(){\n";
  response_message +="document.getElementById(\"mqtt_c\").innerHTML = req5.responseText;\n";
  response_message +=" }\n";
  response_message +=" req5.send();\n";
  response_message +="}\n";
  response_message +="</script>";
  
  response_message +="</head>";
  response_message += "<body><center><div class=\"blockk\">";
  response_message += " Интерфейс IoT Контроллера увлажнителя<br><hr>";
  //time display
  response_message += "<b>ID устройства: "+String(ESP.getChipId())+" </b><hr>";
  //response_message += "<body><center>"; 
  //time
  int times =(millis()/1000);
  int timehour =(((times)  % 86400L) / 3600);
    if ( ((times % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      int timehour = 0;
               }
    int timeminuts=((times  % 3600) / 60); // print the minute (3600 equals secs per minute) 
    if ( (times % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      int timeminuts = 0;
        }
    int timeseconds=(times % 60); // print the second
   
   response_message += "<hr>Время работы модуля: <div id=\"xz\">00:00:00</div>";   

  
  //response_message += "<center>Время работы модуля: "+String((millis()/1000))+" секунд. <br></center>";
  //time display
  response_message += "<hr>Cтатус: ";
  if (digitalRead(hum_level_low)){
  response_message += "<br><hr>Увлажнитель: Включен";
    //button
  response_message += "<br><hr>";
  response_message += "<style>";
  response_message += "a.button9 {";
  response_message += " position: relative;";
  response_message += " display: inline-block;";
  response_message += " color: #ff1a33;";
  response_message += " font-weight: bold;";
  response_message += " text-decoration: none;";
  response_message += "text-shadow: rgba(255,255,255,.5) 1px 1px, rgba(100,100,100,.3) 3px 7px 3px;";
  response_message += "user-select: none;";
  response_message += "padding: 1em 2em;";
  response_message += "outline: none;";
  response_message += "border-radius: 3px / 100%;";
  response_message += "background-image:";
  response_message += "linear-gradient(45deg, rgba(255,255,255,.0) 30%, rgba(255,255,255,.8), rgba(255,255,255,.0) 70%),";
  response_message += "linear-gradient(to right, rgba(255,255,255,1), rgba(255,255,255,0) 20%, rgba(255,255,255,0) 90%, rgba(255,255,255,.3)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5));";
  response_message += "background-repeat: no-repeat;";
  response_message += "background-size: 200% 100%, auto, 100% 2px, 100% 2px, 100% 1px, 100% 1px;";
  response_message += "background-position: 200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "box-shadow: rgba(0,0,0,.5) 3px 10px 10px -10px;";
  response_message += "}";
  response_message += "a.button9:hover {";
  response_message += "transition: .5s linear;";
  response_message += "background-position: -200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "}";
  response_message += "a.button9:active {";
  response_message += " top: 1px;";
  response_message += "}";
  
  response_message += "button.button9 {";
  response_message += " position: relative;";
  response_message += " display: inline-block;";
  response_message += " color: #ff1a33;";
  response_message += " font-weight: bold;";
  response_message += " text-decoration: none;";
  response_message += "text-shadow: rgba(255,255,255,.5) 1px 1px, rgba(100,100,100,.3) 3px 7px 3px;";
  response_message += "user-select: none;";
  response_message += "padding: 1em 2em;";
  response_message += "outline: none;";
  response_message += "border-radius: 3px / 100%;";
  response_message += "background-image:";
  response_message += "linear-gradient(45deg, rgba(255,255,255,.0) 30%, rgba(255,255,255,.8), rgba(255,255,255,.0) 70%),";
  response_message += "linear-gradient(to right, rgba(255,255,255,1), rgba(255,255,255,0) 20%, rgba(255,255,255,0) 90%, rgba(255,255,255,.3)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5));";
  response_message += "background-repeat: no-repeat;";
  response_message += "background-size: 200% 100%, auto, 100% 2px, 100% 2px, 100% 1px, 100% 1px;";
  response_message += "background-position: 200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "box-shadow: rgba(0,0,0,.5) 3px 10px 10px -10px;";
  response_message += "}";
  response_message += "button.button9:hover {";
  response_message += "transition: .5s linear;";
  response_message += "background-position: -200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "}";
  response_message += "button.button9:active {";
  response_message += " top: 1px;";
  response_message += "}";
  response_message += "</style>";
  response_message += "<a href=\"/powerS\" class=\"button9\">Отключить</a>";
  //mode form
  response_message += "<form action=\"set_mode.html\"><p>Режим работы:</p><div>";
  response_message += "<input type=\"radio\" id=\"mode1\" name=\"mode\" value=\"1\" "+get_mode(1)+">";
  response_message += "<label for=\"mode1\">Режим 1</label>";
  response_message += "<input type=\"radio\" id=\"mode2\" name=\"mode\" value=\"2\" "+get_mode(2)+">";
  response_message += "<label for=\"mode2\">Режим 2</label>";
  response_message += "<input type=\"radio\" id=\"mode3\" name=\"mode\" value=\"3\" "+get_mode(3)+">";
  response_message += "<label for=\"mode3\">Режим 3</label>";
  response_message += "</div><div><button class=\"button9\" type=\"submit\">Выбрать</button></div></form>";
  
  }else{
  response_message += "<br><hr>Увлажнитель: Выключен";
  response_message += "<br><hr>";
  response_message += "<style>";
  response_message += "a.button9 {";
  response_message += " position: relative;";
  response_message += " display: inline-block;";
  response_message += " color: #1ca949;";
  response_message += " font-weight: bold;";
  response_message += " text-decoration: none;";
  response_message += "text-shadow: rgba(255,255,255,.5) 1px 1px, rgba(100,100,100,.3) 3px 7px 3px;";
  response_message += "user-select: none;";
  response_message += "padding: 1em 2em;";
  response_message += "outline: none;";
  response_message += "border-radius: 3px / 100%;";
  response_message += "background-image:";
  response_message += "linear-gradient(45deg, rgba(255,255,255,.0) 30%, rgba(255,255,255,.8), rgba(255,255,255,.0) 70%),";
  response_message += "linear-gradient(to right, rgba(255,255,255,1), rgba(255,255,255,0) 20%, rgba(255,255,255,0) 90%, rgba(255,255,255,.3)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(125,125,125,1), rgba(255,255,255,.9) 45%, rgba(125,125,125,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5)),";
  response_message += "linear-gradient(to right, rgba(223,190,170,1), rgba(255,255,255,.9) 45%, rgba(223,190,170,.5));";
  response_message += "background-repeat: no-repeat;";
  response_message += "background-size: 200% 100%, auto, 100% 2px, 100% 2px, 100% 1px, 100% 1px;";
  response_message += "background-position: 200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "box-shadow: rgba(0,0,0,.5) 3px 10px 10px -10px;";
  response_message += "}";
  response_message += "a.button9:hover {";
  response_message += "transition: .5s linear;";
  response_message += "background-position: -200% 0, 0 0, 0 0, 0 100%, 0 4px, 0 calc(100% - 4px);";
  response_message += "}";
  response_message += "a.button9:active {";
  response_message += "top: 1px;";
  response_message += "}";
  response_message += "</style>";
  response_message += "<a href=\"/powerS\" class=\"button9\">Включить</a>";

     }
  response_message += "<hr> MQTT статус: <div id=\"mqtt\">отключен</div> ";
  response_message += "<hr> MQTT call: <div id=\"mqtt_c\">отключен</div> ";     
  response_message += "<hr>Температура: <div id=\"xz1\">0.00 А</div> ";
  response_message += "<hr>Влажность: <div id=\"xz2\">0.00 В</div> ";
  response_message += "<hr>Уровень воды: <div id=\"xz3\">0.00 Вт</div> ";
 
  if (WiFi.status() == WL_CONNECTED){
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    response_message += "<hr>Статус: подключен к сети "+String(WiFi.SSID())+"<br>";
    response_message += "Уровень сигнала: "+String(WiFi.RSSI())+" dBi <br><hr>";
    response_message += "IP адрес подключения: "+String(ipStr)+"<br><hr>";
    }else{response_message += "<br><hr>WLAN статус: Отключено<br><hr>"; }
  
  response_message += "<a href=\"/wlan_config\">Настройки беспроводного соединения</a><br><hr>";
  response_message += "<a href=\"/update\">Обновление прошивки (OTA)</a><br><hr>";
  //response_message += "<a href=\"/about\">О разработчике системы</a><br><hr>";
  //response_message += "<li><a href=\"/gpio\">Управление GPIO </h4></li>";
  //response_message += "<li><a href=\"/time\">Синхронизация времени </h4></li></ul>";
  //response_message += "<li><a href=\"/setting\">Запись данных </h4></li></ul>";
  //response_message += "<li><a href=\"/read\">Чтение данных </h4></li></ul>";
 // response_message += "<li><a href=\"/reboot\">Перезагрузка устройства </h4></li></ul>";
  response_message += "</div></center></body></html>";
  
  server.send(200, "text/html", response_message);
}


void wlanPageHandler(){
  if (server.hasArg("ssid")){    
    if (server.hasArg("password")) { WiFi.begin(server.arg("ssid").c_str(), server.arg("password").c_str()); save_EEPROM (server.arg("ssid").c_str(), 2); save_EEPROM (server.arg("password").c_str(), 42); }
    else { WiFi.begin(server.arg("ssid").c_str()); save_EEPROM (server.arg("ssid").c_str(), 2); }
    while (WiFi.status() != WL_CONNECTED){ delay(500); }   
    delay(500);
  }
  
  String response_message = "";
  response_message +="<head>";
  response_message +="<title>Wi-Fi конфигурация</title>";
  response_message += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8;\" />";
  response_message += "<style type=\"text/css\">body{background-color: #e1ff00;color:#FFF;}a {color:#e1ff00;}.blockk {border:solid 1px #2d2d2d;text-align:center;background:#008b8b;padding:10px 10px 10px 10px;-moz-border-radius: 0px;-webkit-border-radius: 0px;border-radius: 0px;}";
  response_message += ".blockk{border:double 2px #000000;-moz-border-radius: 0px;-webkit-border-radius: 0px;border-radius: 0px;}";
  response_message +="</style><style type=\"text/css\" media=\'(min-width: 810px)\'>body{font-size:18px;}.blockk {width: auto;}</style>";
  response_message +="<style type=\"text/css\" media=\'(max-width: auto) and (orientation:landscape)\'>body{font-size:8px;}</style></head>";
  response_message += "<body><center><div class=\"blockk\">";
  response_message += "Настройка беспроводного соединения<br><hr>";
  
  if (WiFi.status() == WL_CONNECTED){
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    response_message += "Статус: Модуль подключен к сети "+String(WiFi.SSID())+"<br><hr>";
    response_message += "Уровень сигнала: "+String(WiFi.RSSI())+" dBi <br><hr>";
    response_message += "IP адрес подключения: "+String(ipStr)+"<br><hr>"; }
    else { response_message += "Статус: Модуль отключен от сети<br><hr>"; }
  
  response_message += "<p>Для подключения к  WiFi сети, пожалуйста выберите сеть...</p><br><hr>";
  int ap_count = WiFi.scanNetworks();
  
  if (ap_count == 0){ response_message += "Не найдено ниодной беспроводной сети.<br><hr>";}
  else {
    response_message += "<form method=\"get\">";
    // Show access points
    for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++) {
      response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      response_message += String(WiFi.SSID(ap_idx)) + " [Уровень сигнала: " + WiFi.RSSI(ap_idx) +" dBi]";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "[защищена]";
      response_message += "<br><br>";
    }    
    response_message += "WiFi пароль доступа (если сеть защищена):<br>";
    response_message += "<input type=\"text\" name=\"password\"><br><hr>";
    response_message += "<input type=\"submit\" value=\"Подключиться\">";
    response_message += "</form>";
  }
  response_message += "</body></html>";
  response_message += "<a href=\"/\">Вернуться назад</a><br><hr>";
  server.send(200, "text/html", response_message);
}

void handleNotFound()
{
  String message = "Файл не найден\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}
void SSDP_init(){
   String ipStr;
     if (WiFi.status() == WL_CONNECTED){
         IPAddress ip = WiFi.localIP();
          ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        }else{
         ipStr = "192.168.4.1";
       }
         SSDP.setSchemaURL("description.xml");
         SSDP.setHTTPPort(80);
         SSDP.setName("IoT Smart Hum Controller");
         SSDP.setSerialNumber(String(ESP.getChipId()));
         SSDP.setURL("http://"+ipStr+"/");
         SSDP.setModelName("IoT Smart Hum Controller");
         SSDP.setModelNumber("1.1.4");
         SSDP.setModelURL("https://cyberex.online");
         SSDP.setManufacturer("V.G.C. Smart Electronics");
         SSDP.setManufacturerURL("https://cyberex.online");
         SSDP.begin();
}
           
void HTTP_init(){
         server.on("/index.html", HTTP_GET, [](){
         server.send(200, "text/plain", "IoT Smart Hum Controller");
             });
         server.on("/description.xml", HTTP_GET, [](){
               SSDP.schema(server.client());
             });
         server.begin();
}

void get_DHT22_data(){
    long interval = 1000;
      if(millis() - prevMills > interval) {
          humidity = dht.getHumidity();
          temperature = dht.getTemperature();
          prevMills = millis();
      }
  
}

void testpage(){
    String response_message = "";
    int times =(millis()/1000);
    int timehour =(((times)  % 86400L) / 3600);
    if (((times % 3600) / 60) < 10 ) { int timehour = 0;}
    int timeminuts=((times  % 3600) / 60); //
    if ( (times % 60) < 10 ) { int timeminuts = 0; }
    int timeseconds=(times % 60); //
    String timenow = String(timehour)+":"+twoDigits(timeminuts)+":"+twoDigits(timeseconds);
    response_message += String(timenow);
    server.send(200, "text/html", response_message);
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
void json(){
   String response_message ="{ \n";
  response_message += "\"chanel1\":";
  if(hum_level_low_b){
   response_message += "\"On\",\n";
  } else {
   response_message  += "\"Off\",\n";
  }
  response_message += "\"temp\":";
  response_message += "\""+String(temperature)+"\", \n";
  response_message += "\"hum\":";
  response_message += "\""+String(humidity)+"\", \n";
  response_message += "\"level1\":";
  response_message += "\""+String(hum_level_low_b)+"\", \n";
  response_message += "\"level2\":";
  response_message += "\""+String(hum_level_medium_b)+"\", \n";
  response_message += "\"level3\":";
  response_message += "\""+String(hum_level_max_b)+"\", \n";
  response_message += "\"water\":";
  response_message += "\""+String(water_lvl_b)+"\" \n";
  response_message += "}";
  server.send(200, "text", response_message);
}
void powerS(){
  String response_message ="";
  if(hum_level_low_b){
      digitalWrite(power_control, HIGH);
      delay(5000) ; 
      digitalWrite(power_control, LOW);
     response_message += "Off";
   }else{
      digitalWrite(power_control, HIGH);
      delay(500) ; 
      digitalWrite(power_control, LOW);
      response_message  += "On";
  }
   response_message +="<meta http-equiv=\"refresh\" content=\"1;URL=/\">";
   server.send(200, "text/html", response_message);
}
void set_mode(){
  String response_message ="";
  if(server.hasArg("mode")){
    String d = server.arg("mode").c_str();
    set_mode_pin(d.toInt());
    response_message += server.arg("mode").c_str();
   }
  response_message +="<meta http-equiv=\"refresh\" content=\"3;URL=/\">";
  server.send(200, "text/html", response_message);
}
void temp(){
    String response_message = "";
    response_message += String(temperature)+ "°C";
    server.send(200, "text/html", response_message);
}

void hum(){
    String response_message = "";
    response_message += String(humidity)+ "%";
    server.send(200, "text/html", response_message);
}
void lew_w(){
    String response_message = "";
    if(water_lvl_b){
      response_message = "минимальный";
    }else{
      response_message = "нормальный";
    }
    server.send(200, "text/html", response_message);
}

   // Чтение данных их ячейки
   String read_EEPROM (int star_t){
            String data;
            int end_t = star_t + 32;
            for(int i = star_t; i < end_t; ++i){
              int bu = EEPROM.read(i);
              if (bu > 31 && bu < 241){
            data += char(bu);
              }
              }
            return data;
         }
         
    // запись данных в ячейки    
    String save_EEPROM (String datas, int cell){
          int end_cell = cell + 32;
          for (int i = cell; i < end_cell; ++i)                   // стираем данные перед записью
          { 
            EEPROM.write(i, 32); 
            }
          for (int i = 0; i < end_cell; ++i)               // записываем данные в ячейки
            {
            EEPROM.write(cell+i, datas[i]);
            }  
            EEPROM.commit();     
         return datas;        
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
    }
 }

void MQTT_send(){
   client.loop();
   unsigned long now = millis();
  if (now - lastMsg > 2000) {
    if(WiFi.status() == WL_CONNECTED) {
     String json ="{ \n";
            json += "\"chanel1\":";
      if(hum_level_low_b){
           json += "\"On\",\n";
        } else {
           json += "\"Off\",\n";
        }
           json += "\"temp\":";
           json += "\""+String(temperature)+"\", \n";
           json += "\"hum\":";
           json += "\""+String(humidity)+"\", \n";
           json += "\"level2\":";
           json += "\""+String(hum_level_medium_b)+"\", \n";
           json += "\"level3\":";
           json += "\""+String(hum_level_max_b)+"\", \n";
           json += "\"water\":";
           json += "\""+String(water_lvl_b)+"\" \n";
           json += "}";
    if(client.connected()){
       client.subscribe("humiditer/control");
       client.publish("humiditer/jsondata", json.c_str());
     }else{
        if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)){
          client.subscribe("humiditer/control");
          client.publish("humiditer/jsondata", json.c_str());
        }
       }
     }
   lastMsg = now; 
  }
 }
 void MQTT_fast_send(){
      if(WiFi.status() == WL_CONNECTED) {
     String json ="{ \n";
            json += "\"chanel1\":";
      if(hum_level_low_b){
           json += "\"On\",\n";
        } else {
           json += "\"Off\",\n";
        }
           json += "\"temp\":";
           json += "\""+String(temperature)+"\", \n";
           json += "\"hum\":";
           json += "\""+String(humidity)+"\", \n";
           json += "\"level2\":";
           json += "\""+String(hum_level_medium_b)+"\", \n";
           json += "\"level3\":";
           json += "\""+String(hum_level_max_b)+"\", \n";
           json += "\"water\":";
           json += "\""+String(water_lvl_b)+"\" \n";
           json += "}";
    if(client.connected()){
       client.subscribe("humiditer/control");
       client.publish("humiditer/jsondata", json.c_str());
     }else{
        if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)){
          client.subscribe("humiditer/control");
          client.publish("humiditer/jsondata", json.c_str());
        }
       }
     }
 }
 void callback(char* topic, byte* payload, unsigned int length) {
           String message;
         for (int i = 0; i < length; i++) 
           {
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
          callbackq = message;
 }

void MQTT_status(){
    String response_message = "";
    if(client.connected()){
      response_message = "подключен";
    }else{
      response_message = "отключен";
    }
    server.send(200, "text/html", response_message);
}

void MQTT_callback(){
    server.send(200, "text/html", callbackq);
}
