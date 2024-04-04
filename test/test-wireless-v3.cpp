#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include "RBS.h"
#include "IoT-Board-NT.h"
#include "wireless.h"
#include "mqtt.h"
#include "rtc.h"

/*WIFI AND MQTT INFORMATION*/
/*========================================================*/
#define MQTT_SERVER "mosquitto.network-telemetrix.com"
#define MQTT_PORT 8883
#define MQTT_CLIENT_USER "Claro-IoT"
#define MQTT_CLIENT_PASS "Claro.2023"
#define MQTT_ID "ESP32-Claro"
#define MQTT_TOPIC_SUB "/claro/control"
#define MQTT_TOPIC_PUB "/claro/monitoring"

const char* ssid = "Claro-Ecuador";
const char* password = "Claro.2023";
//const char* ssid = "Wifi Octavio Indoor 2G";    
//const char* password = "199905498";  
/*========================================================*/

const char* mqtt_server = MQTT_SERVER;
uint16_t mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_CLIENT_USER;
const char* mqtt_password = MQTT_CLIENT_PASS;
const char* mqtt_topic_pub = MQTT_TOPIC_PUB;
const char* mqtt_topic_sub = MQTT_TOPIC_SUB;
String mqtt_ID = MQTT_ID;

WiFiClientSecure WIFIClient;
MQTT mqtt(WIFIClient);

int bitRate = 115200;
void serial_setup(int bitRate){
    Serial.begin(bitRate);
    while(not Serial)
       delay(500); 
    Serial.println(F("\nSerial Port OK!!!"));
    delay(500);
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nBROKER: %s",mqtt.server());
  Serial.printf("\nTOPIC: %s",topic);
  Serial.printf("\nRECEIVED MESSAGE:\n");
  DynamicJsonDocument doc(2048);
  deserializeJson(doc,(const byte*)payload,length);
  serializeJson(doc,Serial);
  Serial.println();
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
  mqtt.setServer(mqtt_server);
  mqtt.setPort(mqtt_port);
  mqtt.setUser(mqtt_user);
  mqtt.setPassword(mqtt_password);
  mqtt.setTopicPUB(mqtt_topic_pub);
  mqtt.setTopicSUB(mqtt_topic_sub);
  mqtt_ID += " (" + String(WiFi.macAddress()) + ")";
  mqtt.setId(mqtt_ID.c_str());

  if(wifi_connect(ssid,password,5)){
    WIFIClient.setInsecure();
    if(mqtt.connect(1)){
      //mqtt.client.setCallback(callback);
      mqtt.subscribe();
      const char* welcome = ("Hello I am " + mqtt_ID).c_str();
      mqtt.client.publish(mqtt_topic_pub,welcome);
    }
  }  
}

void loop() {
  if(wifi_check_connection(ssid,password,4)){
    if(mqtt.checkConnection(1)){
      mqtt.publish(true,rbs.toJson());
      //mqtt_publish(true,rbs.toJson(),mqtt.client,2048,mqtt.topicPUB());
    }
    if(!WIFIClient.connected())
      Serial.printf("\nFAILURE Conection to %s",mqtt.server()); 
  }
  delay(1000);
  mqtt.client.loop();  
}





