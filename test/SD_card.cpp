#include <Arduino.h>
#include <SPI.h>
#include <rtc.h>
#include "SD_card.h"
#include "monitoring.h"

const char* folder = "/RBS";
const char* file = "alarms.json";
String msg = "IMPRIMIR ESTE MENSAJE";

const int ChipSelect = 5;
bool SD_Initialized = false;

void setup(){
  Serial.begin(115200);
  SD_Initialized = SD_card_setup(SD,folder,file,ChipSelect,"{\"Controller\":\"Restar from SETUP\"},\n");
  Serial.println("Reset ESP");
}

void loop(){
  
  bool event = true;
  bool save = true;
  uint8_t cardType = SD.cardType();
    /*==============================================================================*/
    if(cardType == CARD_UNKNOWN)    Serial.println("\nSD Card Type: UNKNOWN");
    else if(cardType == CARD_NONE)  Serial.println("\nSD Card Type: NONE");

    else if(cardType == CARD_MMC)   Serial.println("\nSD Card Type: MMC");
    else if(cardType == CARD_SD)    Serial.println("\nSD Card Type: SDSC");
    else if(cardType == CARD_SDHC)  Serial.println("\nSD Card Type: SDHC");
    /*==============================================================================*/
  save_SD_card(save,event,SD_Initialized,SD,folder,file,msg);
  delay(1000);
}
