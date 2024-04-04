#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include "RBS.h"
#include "IoT-Board-NT.h"
#include "wireless.h"
#include "rtc.h"

#define MOSQUITTO_SERVER "mosquitto.network-telemetrix.com"
#define EMQX_SERVER "emqx.network-telemetrix.com"
#define MQTTS_PORT 8883

/*==========CHANGE THIS FOR EVERY RBS==========*/
#define MQTT_CLIENT_USER "Claro-IoT"
#define MQTT_CLIENT_PASS "Claro.2023"
#define MQTT_TOPIC_SUB "/claro/control"
#define MQTT_TOPIC_PUB "/claro/monitoring"
/*=============================================*/

//const char* ssid = "Claro-Ecuador";
//const char* password = "Claro.2023";
const char* ssid = "Wifi Octavio Indoor 2G";    
const char* password = "199905498";  
String mqtt_ID = "ESP32-Claro ";

const char* mqtt_server = EMQX_SERVER;
const char* mqtt_user = MQTT_CLIENT_USER;
const char* mqtt_password = MQTT_CLIENT_PASS;
const char* mqtt_topic_pub = MQTT_TOPIC_PUB;
const char* mqtt_topic_sub = MQTT_TOPIC_SUB;
uint16_t mqtt_port = MQTTS_PORT;

WiFiClientSecure WIFIClient;
MQTTPubSubClient MQTTClient;    //Changed at MQTTPubSubClient: using MQTTPubSubClient = arduino::mqtt::PubSubClient<1024>;

int bitRate = 115200;
void serial_setup(int bitRate){
    Serial.begin(bitRate);
    while(not Serial)
       delay(500); 
    Serial.println(F("\nSerial Port OK!!!\n"));
    delay(500);
}

/*================DATA TIPO OBJECT PARA ENVIAR A MQTT SERVER=====================*/
  Energia energia;
  Rectifier rectificador(1,50);
  Battery DC(2,100);
  PowerSystem power_system(energia,rectificador,DC);
  TTA tta;
  Combustible combust;
  Controller controlador_GEN;
  Battery bat_GEN(1,80);
  GEN generador(energia,tta,combust,controlador_GEN,bat_GEN);
  RBS rbs(energia,generador,power_system);
/*===============================================================================*/
void setup() {
    serial_setup(bitRate);
    wifi_connect(ssid,password,3);
    mqtt_wifi_connect(WIFIClient,MQTTClient,mqtt_server,mqtt_port,3);
    mqtt_broker_connect(MQTTClient,mqtt_ID,mqtt_user,mqtt_password,2);

    /*
    //subscribe callback which is called when every packet has come
    MQTTClient.subscribe([](const String& topic, const char* payload, const size_t size) {
        Serial.printf("\nTOPIC: %s",topic);
        Serial.printf("\nALL MESSAGE Received:\n");
        DynamicJsonDocument doc(2048);
        deserializeJson(doc,(const char*)payload,size);
        serializeJson(doc,Serial);
        Serial.println();
    });
    */
    

    //subscribe topic and callback which is called when /hello has come
    MQTTClient.subscribe(mqtt_topic_pub, [](const char* payload, const size_t size) {
        Serial.printf("\nTOPIC: %s",mqtt_topic_sub);
        Serial.printf("\nMESSAGE Received:\n");
        DynamicJsonDocument doc(2048);
        //deserializeJson(doc,(const char*)payload,size);
        //serializeJson(doc,Serial);
        Serial.println();
    });   
}

void loop() {
    MQTTClient.update();  // should be called
    if(wifi_validate_connect(ssid,password,4)){
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 2000) {
            prev_ms = millis();
            MQTTpublish(true,rbs,MQTTClient,mqtt_topic_pub);
        }
    }
    delay (1000);
}
