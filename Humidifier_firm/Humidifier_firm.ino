/* Устройство для управления увлажнителем
 *  
 *  © CYBEREX Tech, 2020
 * 
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266SSDP.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <Wire.h>
#include "const_js.h"



#define power_control 5                // Пин управления питанием
#define hum_level_low 12               // Пин уровня мощности низкий
#define hum_level_medium 14            // Пин уровня мощности средний
#define hum_level_max 16               // Пин уровня мощности максимальный
#define water_lvl 13                   // Пин минимального уровня воды
#define DHT22_pin 4                    // Пин датчика температуры и влажности
DHTesp dht;

// WEBs
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
IPAddress apIP(10, 10, 20, 1);
IPAddress netMsk(255, 255, 255, 0);
ESP8266HTTPUpdateServer httpUpdater;

// DNS server
const byte             DNS_PORT = 53;
DNSServer              dnsServer;

// Текущий статус WLAN
unsigned int status = WL_IDLE_STATUS;

// Переменные для хранения статуса сетевого подключения
boolean connect;
boolean wi_fi_st;

boolean stat_reboot, led_stat, stat_wifi; 

// Переменные используемые для CaptivePortal
const char *myHostname  = "Cyberex_Humidifier";            // Имя создаваемого хоста Cyberex_Humidifier.local 
const char *softAP_ssid = "CYBEREX-Humidifier";            // Имя создаваемой точки доступа Wi-Fi

String version_code = "CH-1.6.01.13";

// Переменная для хранения времени последней попытки сетевого подключения
uint32_t lastConnectTry;
// Другие таймеры
uint32_t reboot_t;
uint32_t t10s,  t60s, t5min, t60min, previousMillis;
float humidity = 0.0;
float temperature = 0.0;
boolean hum_level_low_b, hum_level_medium_b, hum_level_max_b, water_lvl_b;
boolean hum_level_low_p, hum_level_medium_p, hum_level_max_p, water_lvl_p;
//timers
uint32_t low_t, med_t, max_t, water_t, prevMills, lastMsg;

int count_rf = 0;

// Структура для хранения токенов сессий 
struct Token {
    String value;
    long created;
    long lifetime;
};

Token   tokens[20];                    // Длина массива структур хранения токенов 

#define TOKEN_LIFETIME 6000000         // время жизни токена в секундах

#define MAX_STRING_LENGTH 30           // Максимальное количество символов для хранения в конфигурации

String ch_id = String(ESP.getChipId()); // Идентификатор устройства

// Структура для хранения конфигурации устройства
struct {
     boolean mqtt_en;
     int     mqtt_time;
     int     statteeprom; 
     int     counter_coeff;
     float   total;
     char    mySSID[MAX_STRING_LENGTH];
     char    myPW[MAX_STRING_LENGTH];
     char    mqtt_serv[MAX_STRING_LENGTH];
     String  mqtt_user;
     String  mqtt_passw;
     String  mqtt_id;
     String  mqtt_topic;
     String  passwd;    
  } settings;


int  CP10s, CP1m, CP5m, CP60m;                                      // Накопление импульсов счетчика 10с/1м/5м/60м 
int  bCP10s, bCP1m, bCP5m, bCP60m;                                  // Накопление импульсов счетчика 10с/1м/5м/60м буфер
float val10s, val1m, val5m, val60m;                                 // Накопленные значения расхода 10с/1м/5м/60м 
long count_total;                                                   // Общее количество импульсов счетчика

void setup() {
    EEPROM.begin(sizeof(settings));                                 // Инициализация EEPROM в соответствии с размером структуры конфигурации      
    pinMode(power_control, OUTPUT);
    dht.setup(DHT22_pin, DHTesp::DHT22);
    read_config();                                                  // Чтение конфигурации из EEPROM
    check_clean();                                                  // Провека на запуск устройства после первичной прошивки
     if(String(settings.passwd) == ""){   
        settings.passwd = "admin";                                  // Если  переменная settings.passwd пуста, то назначаем пароль по умолчанию
     }
     WiFi.mode(WIFI_STA);                                           // Выбираем режим клиента для сетевого подключения по Wi-Fi                
     WiFi.hostname("Humidifier [CYBEREX TECH]");                    // Указываеем имя хоста, который будет отображаться в Wi-Fi роутере, при подключении устройства
     if(WiFi.status() != WL_CONNECTED) {                            // Инициализируем подключение, если устройство ещё не подключено к сети
           WiFi.begin(settings.mySSID, settings.myPW);
      }

     for(int x = 0; x < 60; x ++){                                  // Цикл ожидания подключения к сети (30 сек)
          if (WiFi.status() == WL_CONNECTED){ 
                stat_wifi = true;
                break;
           }
               delay(500); 
      
          }

     if(WiFi.status() != WL_CONNECTED) {                             // Если подключение не удалось, то запускаем режим точки доступа Wi-Fi для конфигурации устройства
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAPConfig(apIP, apIP, netMsk);
            WiFi.softAP(softAP_ssid);
            stat_wifi = false;
        }
        
        delay(2000);
        // Запускаем DNS сервер
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", apIP);
        // WEB страницы
        server.on("/", page_process);
        server.on("/login", handleLogin);
        server.on("/script.js", reboot_devsend);
        server.on("/style.css", css);
        server.on("/index.html", HTTP_GET, [](){
        server.send(200, "text/plain", "IoT Radiation Sensor"); });
        server.on("/description.xml", HTTP_GET, [](){SSDP.schema(server.client());});
        server.on("/inline", []() {
        server.send(200, "text/plain", "this works without need of authentification");
        });
        server.onNotFound(handleNotFound);
        // Здесь список заголовков для записи
        const char * headerkeys[] = {"User-Agent", "Cookie"} ;
        size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
        // Запускаем на сервере отслеживание заголовков 
        server.collectHeaders(headerkeys, headerkeyssize);
        server.begin();
        connect = strlen(settings.mySSID) > 0;                               // Статус подключения к Wi-Fi сети, если есть имя сети
        SSDP_init();
              
}

void loop() {
        portals();
        dnsServer.processNextRequest();
        server.handleClient();  
        reboot_dev_delay();
        get_DHT22_data();
        pin_pr();
        MQTT_send();
        
}

void reboot_devsend(){
   server.send(200, "text", reb_d);
}
