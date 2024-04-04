#include <Arduino.h>
#include <RadioBaseStation.h>
#include <IoT-Board-NT.h>
#include <wireless.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <rtc.h>
#include <SD_card.h>

#define IN_ALM_ENERGIA IN_1
#define IN_ALM_LOW_BAT_VOLT IN_2
#define IN_ALM_TTA IN_3
#define IN_ALM_GEN IN_4
#define OUT_ALM_GEN RELAY_OUT_1
#define OUT_ALM_TTA RELAY_OUT_2
#define PIN_CONTROL_GEN RELAY_OUT_3
#define CS GPIO_NUM_5
#define DEBOUNCE 250
#define DELAY_TIME 1000

/*Configurar por cada RBS*/
/*================================*/
#define RBS_NAME "RBS-Name"
#define RECT_QUANTITY 3
#define RECT_CAPACITY 50
#define BAT_QUANTITY 2
#define BAT_CAPACITY 100
#define PATH_FOLDER "/RBS-001"
#define SD_FILE "alarms.json"
/*================================*/


/*Creacion de los objetos de la RBS*/
/*==========================================================*/
Energia energia;
Rectifier rectificador(RECT_QUANTITY,RECT_CAPACITY);
Battery DC(BAT_QUANTITY,BAT_CAPACITY);
PowerSystem power_system(energia,rectificador,DC);
TTA tta;
Combustible combust;
Controller controlador_GEN;
Battery bat_GEN(1,80);
GEN generador(energia,tta,combust,controlador_GEN,bat_GEN);
RBS rbs(energia,generador,power_system);

const char* ssid = "Claro-Ecuador";
const char* password = "Claro.2023";
const char* ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
String ip;

RTC_DS3231 rtc;
NTPClient timeClient(ntpUDP,ntpServer);
String date_NTP_Str;
String date_RTC_Str;

bool isrEvent = false;
bool save_SD = false;
bool SD_Initialized = false;
unsigned long lastTime;
const char* path_Folder = PATH_FOLDER;
const char* SD_File = SD_FILE;

void serial_setup(int bitRate);
bool SD_card_setup(fs::FS &fs, const char* folder, const char* file, const u_int8_t ChipSelect, const char* msg);
void send_control(int pin, boolean control);
void send_alm(int pin, boolean alm);
bool read_alm(int pin);
void RBS_monitoring(RBS& rbs);
String build_data_str_JSON(RBS& rbs);
void save_SD_card(bool& save, bool& event, bool& SD_init, fs::FS &fs, const char* folder, const char* file, String msg);
void print_msg();
void IRAM_ATTR isr();
void isr_disable();
void isr_enable();

void setup() {
    serial_setup(115200);
    if(wifi_connect(ssid,password,4))
        date_NTP_Str = updateRTCFromNTP(rtc,timeClient);   
    SD_Initialized = SD_card_setup(SD,path_Folder,SD_File,CS,"{\"Controller\":\"Reinicio de Controller\"}");
    
    /*Configuracion PIN de entrada y salida de tarjeta*/
    /*===============================================================================*/
    pinMode(IN_ALM_LOW_BAT_VOLT,INPUT_PULLUP);  /*Set Input Alarm: Low Bat Volt*/
    pinMode(IN_ALM_ENERGIA,INPUT_PULLUP);       /*Set Input  Alarm: Energia Publica*/
    pinMode(IN_ALM_TTA,INPUT_PULLUP);           /*Set Input  Alarm: TTA*/  
    pinMode(OUT_ALM_GEN,OUTPUT);                /*Set Output  Alarm: GEN*/
    pinMode(OUT_ALM_TTA,OUTPUT);                /*Set Output  Alarm: TTA*/
    pinMode(PIN_CONTROL_GEN,OUTPUT);            /*Set Output  Control: ON GEN*/
    /*Configurar las interrupciones de entrada*/
    /*=======================================================================*/
    isr_enable(); 

    rbs.setName(RBS_NAME);
    rbs.powerSystem.battery.status.setCode(CHARGED);
}

void loop() { 
    String msg;
    RBS_monitoring(rbs);  
    send_alm(OUT_ALM_GEN,rbs.gen.status.alm());         /*Send GEN alarm to Telecom Equipment*/
    send_alm(OUT_ALM_TTA,rbs.gen.tta.status.alm());     /*Send TTA alarm to Telecom Equipment*/
    send_control(PIN_CONTROL_GEN,rbs.gen.on());         /*Send Signal Control GEN to controller*/
    msg = build_data_str_JSON(rbs);
    isr_disable();
    save_SD_card(save_SD,isrEvent,SD_Initialized,SD,path_Folder,SD_File,msg);
    isr_enable();
    print_msg();

    delay(1*DELAY_TIME);
}

void serial_setup(int bitRate){
    Serial.begin(bitRate);
    while(not Serial)
       delay(500); 
    Serial.println(F("\nSerial Port OK!!!"));
    delay(500);
}

bool SD_card_setup(fs::FS &fs, const char* folder, const char* file, const u_int8_t ChipSelect, const char* msg){
    if(!SD.begin(ChipSelect)){
        Serial.println("SD Card Failed!!!");
        Serial.println();
        return false;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE or cardType == CARD_UNKNOWN){
        Serial.println("SD card Not attached");
        Serial.println();
        return false;
    }
    Serial.println("SD card detected");
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC)        Serial.println("MMC");
    else if(cardType == CARD_SD)    Serial.println("SDSC");
    else if(cardType == CARD_SDHC)  Serial.println("SDHC");
    else                            Serial.println("UNKNOWN");

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %llu MB\n", cardSize);

    listDir(fs,"/",0);
    createDir(fs,folder);
    String str = String(folder) + "/" + String(file);
    const char* pathFile = str.c_str();
    String message = String(msg) + ",";
    Serial.println();
    File tmpFile =  fs.open(pathFile);
    if(!tmpFile){
        writeFile(fs,pathFile,"[");
        Serial.println();
    }
    tmpFile.close();

    appendFile(fs,pathFile,message.c_str());
    //Serial.println();
    //readFile(fs,pathFile);
    
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024)); 
    Serial.println();
    return true; 
}

void send_control(int pin, boolean control){
    digitalWrite(pin,control);  /*Send Signal Control to equipment*/
}

void send_alm(int pin, boolean alm){
    digitalWrite(pin,alm);      /*Send alarm to pin for Telecom Equipment*/
}

bool read_alm(int pin){
    return digitalRead(pin);
}

void RBS_monitoring(RBS& rbs){
    bool tta_alm, gen_alm, ac_alm, lowVolt_alm; 
    bool on_gen;
    bool TTA_code[4], BAT_code[4];
    String code;

    bool A,B,C,D;
    A = read_alm(IN_ALM_ENERGIA);
    B = read_alm(IN_ALM_LOW_BAT_VOLT);
    C = read_alm(IN_ALM_TTA);

    for(int i=1; i<=2; i++){
        D = rbs.gen.on();
        on_gen = (A and B) or (A and D);
        rbs.gen.setOn(on_gen);
    }

    ac_alm = A;
    lowVolt_alm = B; 
    gen_alm = on_gen; 
    tta_alm = C or (A and not D);

    rbs.energy.status.setAlm(ac_alm);
    rbs.powerSystem.battery.status.setAlm(lowVolt_alm);
    rbs.gen.tta.status.setAlm(tta_alm);
    rbs.gen.status.setAlm(gen_alm); 
    
    TTA_code[0] = not A and not D and C;    /*Falla transferencia: GEN-->Energia Publica*/
    TTA_code[1] = (not A and D and C) or (A and not D and not C);   /*Falla Relay*/
    TTA_code[2] = A and D and C;    /*Falla transferencia: Energia Publica-->GEN*/
    TTA_code[3] = A and not D and C;    /*No Energia Publica, NO GEN*/   
    TTA_code[4] = (not A and not C) or (D and not C);    /*TTA OK*/  
  
    if(TTA_code[0])
        code = TTA_CODE_1;
    if(TTA_code[1])
        code = TTA_CODE_2 ;
    if(TTA_code[2])
        code = TTA_CODE_3;
    if(TTA_code[3])
        code = TTA_CODE_4;
    if(TTA_code[4])
        code = OK;
    rbs.gen.tta.status.setCode(code);

    if(ac_alm)
        rbs.energy.status.setCode(NOT_ENERGY);
    else
        rbs.energy.status.setCode(PUBLIC_ENERGY);
    
    if(gen_alm)
        rbs.gen.status.setCode(WORKING);
    else
        rbs.gen.status.setCode(NOT_WORKING);
    
    BAT_code[0] = (A and not D) or (A and C) or (C and not D);   /*Discharging*/
    BAT_code[1] = (not A and B and not C) or (B and not C and D) or (not A and B and D) or (A and not C and D);   /*Discharged & Charging*/
    BAT_code[2] = not A and not B and not C;    /*OK*/

    if(BAT_code[0])
        code = DISCHARGING;
    if(BAT_code[1])
        code = CHARGING;
    if(BAT_code[2])
        code = OK;
    rbs.powerSystem.battery.status.setCode(code);    
  
    DateTime now = getRTC(rtc);
    rbs.energy.status.setTimestamp(dateToString(now));
    rbs.gen.status.setTimestamp(dateToString(now));
    rbs.gen.tta.status.setTimestamp(dateToString(now));
    rbs.powerSystem.battery.status.setTimestamp(dateToString(now));

    rbs.gen.ac.status = rbs.energy.status;
    rbs.powerSystem.ac.status = rbs.energy.status;
}

String build_data_str_JSON(RBS& rbs){
    String msg = "";
    DynamicJsonDocument doc(2048);
    JsonObject json = doc.to<JsonObject>();
    json["RBS"] = rbs.toJson();
    //serializeJsonPretty(doc,msg);
    serializeJson(doc,msg);
    //Serial.println(msg);    
    //Serial.println();
    return msg;;
}

void save_SD_card(bool& save, bool& event, bool& SD_init, fs::FS &fs, const char* folder, const char* file, String msg){
    String str = String(folder) + "/" + String(file);
    const char* pathFile = str.c_str();

    uint8_t cardType = SD.cardType();    
   
    if(!SD_init)
        SD_init = SD_card_setup(SD,folder,file,CS,"{\"SD Card\":\"Inicializando al insertar SD Card\"}");
    
    if(!fs.exists("/")){
        cardType = SD.cardType();
        if(cardType == CARD_UNKNOWN)    Serial.println("\nSD Card Type: UNKNOWN");
        else if(cardType == CARD_NONE)  Serial.println("\nSD Card Type: NONE");

        else if(cardType == CARD_MMC)   Serial.println("\nSD Card Type: MMC");
        else if(cardType == CARD_SD)    Serial.println("\nSD Card Type: SDSC");
        else if(cardType == CARD_SDHC)  Serial.println("\nSD Card Type: SDHC");
        
        Serial.println("SD Card retirada...No se reconoce\n");  
        save = false;
        return;
    }
    
    if(!fs.exists(pathFile)){
        cardType = SD.cardType();
        SD_card_setup(fs,folder,file,CS,"{\"SD Card\":\"Archivos borrados\"}"); 
    }

    if(!save)
        return;

    if(msg.isEmpty())
        return;
        
    if(!event)
        return;

    Serial.println(msg);    
    Serial.println();
    msg = msg + ",";
    appendFile(fs,pathFile,msg.c_str()); 
    save = false;
    event = false;

}

void print_msg(){
    uint8_t cardType = SD.cardType();
    /*==============================================================================*/
    if(cardType == CARD_UNKNOWN)    Serial.println("\nSD Card Type: UNKNOWN");
    else if(cardType == CARD_NONE)  Serial.println("\nSD Card Type: NONE");

    else if(cardType == CARD_MMC)   Serial.println("\nSD Card Type: MMC");
    else if(cardType == CARD_SD)    Serial.println("\nSD Card Type: SDSC");
    else if(cardType == CARD_SDHC)  Serial.println("\nSD Card Type: SDHC");
    /*==============================================================================*/

    Serial.printf(F("===========================\n"));
    Serial.printf(F("STATUS ENERGIA RBS:\n"));
    Serial.printf(F("===========================\n"));
    Serial.printf(F("* Energia --> %s\n"),rbs.gen.ac.status.code().c_str());
    Serial.printf(F("* GEN --> %s\n"),rbs.gen.status.code().c_str());
    Serial.printf(F("* TTA --> %s\n"),rbs.gen.tta.status.code().c_str());
    Serial.printf(F("* BAT --> %s\n"),rbs.powerSystem.battery.status.code().c_str());

    Serial.printf(F("\nALARMAS DE RBS:\n"));
    Serial.printf(F("====================\n"));
    if(rbs.energy.status.alm())     Serial.printf(F("* Energia\n"));
    if(rbs.gen.status.alm())        Serial.printf(F("* GEN\n"));
    if(rbs.gen.tta.status.alm())    Serial.printf(F("* TTA\n"));
    if(rbs.powerSystem.battery.status.alm()) Serial.printf(F("* Low BAT Volt\n"));
    
    if(rbs.gen.on())    Serial.printf(F("\nGEN Working?  YES\n"));
    else                Serial.printf(F("\nGEN Working? --> NOT\n"));
    Serial.println();
}

void IRAM_ATTR isr(){
    isrEvent = true;
    save_SD = true;
    lastTime = millis();
}

void isr_disable(){
    detachInterrupt(digitalPinToInterrupt(IN_ALM_LOW_BAT_VOLT));
    detachInterrupt(digitalPinToInterrupt(IN_ALM_ENERGIA));
    detachInterrupt(digitalPinToInterrupt(IN_ALM_TTA));
}

void isr_enable(){
    attachInterrupt(digitalPinToInterrupt(IN_ALM_LOW_BAT_VOLT),isr,CHANGE);
    attachInterrupt(digitalPinToInterrupt(IN_ALM_ENERGIA),isr,CHANGE);
    attachInterrupt(digitalPinToInterrupt(IN_ALM_TTA),isr,CHANGE); 
}