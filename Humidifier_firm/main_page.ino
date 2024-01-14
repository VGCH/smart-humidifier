void handleRoot() {
   if (captivePortal()) {  
    return;
  }
  String header;
  if (!validateToken()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String html = "<html><head><meta charset=\"UTF-8\"><title>Увлажнитель воздуха</title>";
  html += "<link href=\"style.css\" rel=\"stylesheet\" type=\"text/css\" />";
  html +=  js;
  html += "</head><body>";
  html += divcode;
  html += "<h2> Основные данные увлажнителя</h2>";
  html += "<div class =\"frame\">";
  String content ="";
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  html += "<text>Время работы устройства:</text> <div class =\"live\" id=\"timew\">00:00:00:00</div>";
  html += "<text>Статус MQTT </text> <div class =\"live\" id=\"MQTT\">"+MQTT_status()+"</div>";
  html += "<text>Температура, °C </text> <div class =\"live\" id=\"temp\">0.00</div>";
  html += "<text>Влажность, %Rh </text>  <div class =\"live\" id=\"hum\" >0.00</div>";
  html += "<text>Уровень воды  </text>  <div class =\"live\" id=\"water\" >В норме</div><br><br>";
  html += "<div id=\"pbut\" >"+ bt_st() +"</div><br>";
  html += "<div id=\"radios\" >";
  html +=  radio_b();
  html += "</div>";
  html += "<br></div></body>";
  html += "<center><br><a href=\"/?page=wlan_config\">Wi-Fi конфигурация</a><br>";
  html += "<a href=\"/?page=send_config\">Настройка передачи данных</a><br>";
  html += "<a href=\"/?page=changelp\">Изменение пароля устройства</a><br>";
  html += "<a href=\"/?page=update_fw\">Обновление прошивки</a><br>";
  html += "<a href=\"#\" onclick=\"rebootdev()\">Перезагрузить устройство</a><br>";
  html += "<a href=\"/login?DISCONNECT=YES\">Выход</a></center>";
  html += "<footer>© <b>CYBEREX TECH</b>, 2024. Версия микро ПО <b>"+version_code+"</b>.</footer>";
  html += "<script src=\"script.js\"></script>"; 
  html += "</html>";
  server.send(200, "text/html", html);
}
String bt_st(){
    String st;
  if(hum_level_low_b){;
      st = "<center><a href=\"#\" class=\"btn_on\" onclick=\"on_off()\">Выключить увлажнитель</a></center>";
   }else{
      st = "<center><a href=\"#\" class=\"btn_off\" onclick=\"on_off()\">Включить увлажнитель</a></center>";
  }
  return st;
}
String radio_b(){
  String html  = "<center><text> Режим 1 </text><input onclick=\"radio_b(this)\" type=\"radio\" id=\"mode1\" name=\"mode\" value=\"1\" "+get_mode(1)+">";
         html += "<text> Режим 2 </text><input onclick=\"radio_b(this)\" type=\"radio\" id=\"mode2\" name=\"mode\" value=\"2\" "+get_mode(2)+">";
         html += "<text> Режим 3 </text><input onclick=\"radio_b(this)\" type=\"radio\" id=\"mode3\" name=\"mode\" value=\"3\" "+get_mode(3)+"></center>";
   if(!hum_level_low_b){ html = ""; }
   return html;
}
