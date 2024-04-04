/*Codigo para lectura del tiempo del modulo RTC DS3231 y sincronizacion hacia un server NTP mediante
  una conexion WiFi con SSID: Claro-Ecuador y Password: Claro.2023. La fecha debe ser en formato 
  ISO 8601 ("AAAA-MM-DDTHH:MM:SS")
*/
#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include "rtc.h"

RTC_DS3231 rtc;
WiFiUDP ntpUDP;
const char* ssid = "Claro-Ecuador";
const char* password = "Claro.2023";
const char* ntpServer = "pool.ntp.org";
String ip;

NTPClient timeClient(ntpUDP,ntpServer);
String date_NTP_Str;
String date_RTC_Str;

unsigned long delayTime =1000;

void setup () {
    Serial.begin(115200);
    WiFi.begin(ssid,password);
    Serial.print("\nConnecting to WiFi");
    for (int i=1; i<=6; i++){
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(delayTime);
        }
    }

    if (WiFi.status() == WL_CONNECTED){
        IPAddress ip = WiFi.localIP();
        Serial.println("\nSUSSCEFUL Conection to Wifi!!!");
        Serial.printf(F("* SSID: %s\n"),ssid);
        Serial.printf(F("* Pass: %s\n"),password);
        Serial.printf(F("* IP:   %s\n"),ip.toString().c_str());
        Serial.println();
        date_NTP_Str = updateRTCFromNTP(rtc,timeClient);
    }
    else
        Serial.println("\nWifi NO CONNECTED!!!\n");   
}

void loop() {
    if(rtc.begin()){
        DateTime now = rtc.now(); 
        date_RTC_Str = dateToString(now);
        Serial.printf(F("DATE UPDATE:\n"));
        Serial.printf(F("From RTC module: %s\n"),date_RTC_Str.c_str());
        Serial.printf(F("From NTP Server: %s (Last Update)\n"),date_NTP_Str.c_str());
        Serial.println();  
    }
    else
        Serial.println("Lost Conection with RTC DS3231 module");    
    
    delay(1000);
}

