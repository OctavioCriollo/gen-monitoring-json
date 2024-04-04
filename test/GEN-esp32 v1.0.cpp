#include <Arduino.h>
#include <RadioBaseStation.h>
#include <IoT-Board-NT.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <rtc.h>
#include <SD_card.h>

#define IN_ALM_LOW_BAT_VOLT IN_1
#define IN_ALM_ENERGIA IN_2
#define IN_ALM_GEN IN_3
#define IN_ALM_TTA IN_4
#define OUT_ALM_GEN RELAY_OUT_1
#define OUT_ALM_TTA RELAY_OUT_2
#define PIN_CONTROL_GEN RELAY_OUT_3
#define CS GPIO_NUM_5
#define DEBOUNCE 100
#define DELAY_TIME 1000
#define RBS_NAME "RBS-Name"
#define RECT_QUANTITY 3
#define RECT_CAPACITY 50
#define BAT_QUANTITY 2
#define BAT_CAPACITY 100

#define PATH_FOLDER "/RBS-001"
#define SD_FILE "alarms.json"

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
bool SD_Initialized = false;
unsigned long lastTime;
bool tta_failure,gen_failure,on_gen,gen_alm; 
bool A,B,C,D;
const char* path_Folder = PATH_FOLDER;
const char* SD_File = SD_FILE;
String str = String(path_Folder) + "/" + String(SD_File);
const char* pathFile = str.c_str();

void serial_setup(int bitRate);
bool wifi_setup(const char* ssid, const char* password);
void send_control(int pin, boolean control);
void send_alm(int pin, boolean alm);
bool read_alm(int pin);
void RBS_monitoring(RBS& rbs);
void print_msg();
bool SD_card_setup(fs::FS &fs, const char* folder, const char* file, const u_int8_t ChipSelect, const char* msg);
String dataStrJson(bool& event, unsigned long lastTime, unsigned long transient, RBS& rbs);
void IRAM_ATTR isr();

void setup() {
    serial_setup(115200);
    if(wifi_setup(ssid,password))
        date_NTP_Str = updateRTCFromNTP(rtc,timeClient);   
    SD_Initialized = SD_card_setup(SD,path_Folder,SD_File,CS,"{\"Controller\":\"Reinicio de Controller\"}");
    
    /*Configuracion PIN de entrada y salida de tarjeta*/
    /*===============================================================================*/
    pinMode(IN_ALM_LOW_BAT_VOLT,INPUT_PULLUP);  /*Set Input Alarm: Low Bat Volt*/
    pinMode(IN_ALM_ENERGIA,INPUT_PULLUP);       /*Set Input  Alarm: Energia Publica*/
    pinMode(IN_ALM_GEN,INPUT_PULLUP);           /*Set Input  Alarm: GEN*/
    pinMode(IN_ALM_TTA,INPUT_PULLUP);           /*Set Input  Alarm: TTA*/  
    pinMode(OUT_ALM_GEN,OUTPUT);                /*Set Output  Alarm: GEN*/
    pinMode(OUT_ALM_TTA,OUTPUT);                /*Set Output  Alarm: TTA*/
    pinMode(PIN_CONTROL_GEN,OUTPUT);            /*Set Output  Control: ON GEN*/
    /*Configurar las interrupciones de entrada*/
    /*=======================================================================*/
    attachInterrupt(digitalPinToInterrupt(IN_ALM_LOW_BAT_VOLT),isr,CHANGE);
    attachInterrupt(digitalPinToInterrupt(IN_ALM_ENERGIA),isr,CHANGE);
    attachInterrupt(digitalPinToInterrupt(IN_ALM_GEN),isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(IN_ALM_TTA),isr,CHANGE); 

    rbs.setName(RBS_NAME);
}

void loop() { 
    if(!SD_Initialized)
        SD_Initialized = SD_card_setup(SD,path_Folder,SD_File,CS,"{\"SD Card\":\"Inicializando al insertar SD Card\"}");
    
    RBS_monitoring(rbs);
    send_alm(OUT_ALM_GEN,rbs.gen.status.alm());         /*Send GEN alarm to Telecom Equipment*/
    send_alm(OUT_ALM_TTA,rbs.gen.tta.status.alm());     /*Send TTA alarm to Telecom Equipment*/
    send_control(PIN_CONTROL_GEN,rbs.gen.on());         /*Send Signal Control GEN to controller*/
    print_msg();
    String msg = dataStrJson(isrEvent,lastTime,DEBOUNCE,rbs);

    uint8_t cardType = SD.cardType();    
    if(!SD.exists("/")){
        if(cardType == CARD_UNKNOWN)    Serial.println("\nSD Card Type: UNKNOWN");
        else if(cardType == CARD_NONE)  Serial.println("\nSD Card Type: NONE");
        Serial.println("Revisar SD Card...No se reconoce\n");  
    }
    else
        if(!SD.exists(pathFile))
            SD_card_setup(SD,path_Folder,SD_File,CS,"{\"SD Card\":\"Archivos borrados\"}");  
        else
            if(!msg.isEmpty())
                appendFile(SD,pathFile,msg.c_str());     
    delay(1*DELAY_TIME);
}

void serial_setup(int bitRate){
    Serial.begin(bitRate);
    while(not Serial){
       Serial.println(F("\nConnecting to Serial Port..."));
       delay(0.5*DELAY_TIME); 
    }
    Serial.println(F("\nSerial Port OK!!!\n"));
    delay(0.5*DELAY_TIME);
}

bool wifi_setup(const char* ssid, const char* password){
    WiFi.begin(ssid,password);
    Serial.print("\nConnecting to WiFi");
    for (int i=1; i<=3; i++){
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(DELAY_TIME);
        }
    }
    if (WiFi.status() == WL_CONNECTED){
        IPAddress ip = WiFi.localIP();
        Serial.println("\nSUSSCEFUL Conection to Wifi!!!");
        Serial.printf(F("* SSID: %s\n"),ssid);
        Serial.printf(F("* Pass: %s\n"),password);
        Serial.printf(F("* IP:   %s\n"),ip.toString().c_str());
        Serial.println();
        return true;
    }
    else{
        Serial.println("\nWifi NO CONNECTED!!!\n"); 
        return false;
    }
        
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
    boolean tta_failure,gen_failure,gen_alm,on_gen,ac_alm,lowVolt_alm; 
    boolean A,B,C,D;
    A = read_alm(IN_ALM_GEN);
    B = read_alm(IN_ALM_TTA);
    C = read_alm(IN_ALM_ENERGIA);
    D = read_alm(IN_ALM_LOW_BAT_VOLT);
   
    gen_failure = (A and not C) or (not A and C and D) or (A and C and not D);
    tta_failure = (B and not C) or (B and D) or (not B and C and not D);
    on_gen = (C and D) or (A and C) or (not A and not B and C);
    gen_alm = (A and not C) or (C and D) or (A and C and not D) or (not A and not B and C and not D);
    ac_alm = C;
    lowVolt_alm = D;

    rbs.energy.status.setAlm(ac_alm);
    rbs.energy.status.setTimestamp(getRTC(rtc));
    rbs.powerPlant.battery.status.setAlm(lowVolt_alm);
    rbs.powerPlant.battery.status.setTimestamp(getRTC(rtc));

    if(ac_alm)
        rbs.energy.status.setCode(NOT_ENERGY);
    else
        rbs.energy.status.setCode(WITH_ENERGY);
    
    rbs.gen.ac = rbs.energy;
    rbs.powerPlant.ac = rbs.energy;

    if(lowVolt_alm)
        rbs.powerPlant.battery.status.setCode(DISCHARGED);
    else
        rbs.powerPlant.battery.status.setCode(NOT_DISCHARGED);

    if(tta_failure){
        rbs.gen.tta.status.setCode(FAILURE);
        //rbs.gen.tta.status.setTimestamp(getRTC(rtc));
    }
    else
        rbs.gen.tta.status.setCode(OK);    

    if(gen_failure){
        rbs.gen.status.setCode(FAILURE);
        //rbs.gen.status.setTimestamp(getRTC(rtc));
    }
    else
        if(on_gen){
            rbs.gen.status.setCode(WORKING);
            //rbs.gen.status.setTimestamp(getRTC(rtc));
        }
        else
            rbs.gen.status.setCode(NOT_WORKING);
    
    
    rbs.gen.tta.status.setTimestamp(getRTC(rtc));
    rbs.gen.status.setTimestamp(getRTC(rtc));
    rbs.gen.status.setTimestamp(getRTC(rtc));

    rbs.gen.tta.status.setAlm(tta_failure);
    rbs.gen.status.setAlm(gen_alm);
    rbs.gen.setOn(on_gen);
}

void print_msg(){
    Serial.printf(F("====================\n"));
    Serial.printf(F("STATUS DEL GEN:\n"));
    Serial.printf(F("====================\n"));
    Serial.printf(F("* TTA --> %s\n"),rbs.gen.tta.status.code());
    Serial.printf(F("* GEN --> %s\n"),rbs.gen.status.code());

    Serial.printf(F("\nALARMAS DE RBS:\n"));
    Serial.printf(F("====================\n"));
    if(rbs.energy.status.alm())     Serial.printf(F("* Energia\n"));
    if(rbs.gen.status.alm())        Serial.printf(F("* GEN\n"));
    if(rbs.gen.tta.status.alm())    Serial.printf(F("* TTA\n"));
    if(rbs.powerPlant.battery.status.alm()) Serial.printf(F("* Low BAT Volt\n"));
    
    if(rbs.gen.on())  Serial.printf(F("\nGEN Working?  YES\n"));
    else    Serial.printf(F("\nGEN Working? --> NOT\n"));
    Serial.println();
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
    String pathFile = String(folder) + "/" + String(file);
    String message = String(msg) + ",";
    Serial.println();
    appendFile(fs,pathFile.c_str(),message.c_str());
    //Serial.println();
    //readFile(fs,pathFile.c_str());
    
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024)); 
    Serial.println();
    return true; 
}

String dataStrJson(bool& event, unsigned long lastTime, unsigned long transient, RBS& rbs){
    String msg = "";
    if(event)
        if(millis() - lastTime >= transient){
            event = false; 
            DynamicJsonDocument jsonDoc(2048);
            JsonObject json = jsonDoc.to<JsonObject>();
            json["RBS"] = rbs.toJson();
            //serializeJsonPretty(jsonDoc,msg);
            serializeJson(jsonDoc,msg);
            Serial.println(msg);
            Serial.println();
            msg = msg + ",";
        } 
    return msg;
}

void IRAM_ATTR isr(){
    isrEvent = true;
    lastTime = millis();
}