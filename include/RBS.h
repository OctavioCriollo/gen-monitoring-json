#include <ArduinoJson.h>

#define DEFAULT_TIMESTAMP "2023-08-03T12:34:56z"
#define FAILURE "FAILURE"
#define WORKING "WORKING"
#define NOT_WORKING "OFF"
#define NOT_ENERGY "NOT ENERGY"
#define CHARGED "CHARGED"
#define PUBLIC_ENERGY "ENERGY OK"
#define DISCHARGING "DISCHARGING"
#define CHARGING "CHARGING"
#define DISCHARGED "DISCHARGED"
#define OK "OK"
#define TTA_CODE_1 "TRANSFER FAILURE: GEN-->Energia Publica"
#define TTA_CODE_2 "FAILURE: Falla Relay"
#define TTA_CODE_3 "TRANSFER FAILURE: Energia Publica-->GEN"
#define TTA_CODE_4 "No Energia Publica, NO GEN"

#define ALARM true
#define NOT_ALARM false
#define LOW_BAT_VOLT 46.0
#define HIGH_BAT_VOLT 48.0
#define LOW_DC_VOLT 46.0
#define HIGH_DC_VOLT 48.0
#define LOAD_VOLT_DISCONNECT 43.4   //LVD
#define BAT_VOLT_DISCONNECT 42.3    //BVD
#define LOW_FUEL "LOW FUEL"
#define HIGH_FUEL "HIGH FUEL"
#define MANUAL "MANUAL"
#define AUTOMATIC "AUTOMATIC"
#define ON 1
#define OFF 0

/*Alarm monitoring from Claro*/
#define ALM_ENERGIA_PUBLICA 1       //ALM 1: Alarma Energia Publica
#define ALM_GEN             2       //ALM 7: Alarma GEN
#define ALM_TTA             10      //ALM 10: Alarma TTA
#define ALM_COMBUSTIBLE     15      //ALM 15: Alarma Combustible Bajo
#define ALM_LOW_VOLT        1       //ALM 1: Alarma Bajo Voltaje

/*Class Alarms and Indicator*/
class Status {
private:
    const char* _code;
    boolean _alm;
    const char* _timestamp;   /*ISO 8601 ("AAAA-MM-DDTHH:MM:SS")*/
public:
    /*CONSTRUCTOR Class Status
    ===============================================*/
    Status():
    _code(OK), _alm(NOT_ALARM), _timestamp(DEFAULT_TIMESTAMP) {}

    /*GETTER atributos Class Status
    ===============================================*/
    const char* code() const{
        return _code;
    }
    const boolean alm() const{
        return _alm;
    }
    const char* timestamp() const{
        return _timestamp;
    }

    /*SETTER atributos Class Status
    ===============================================*/
    void setCode(const char* code){
        _code = code;
    }
    void setAlm(boolean alm){
        _alm = alm;
    }
    void setTimestamp(const char* timestamp){
        _timestamp = timestamp;
    }

    /*ARDUINO-JSON: Convert Class Status to JSON
    =================================================*/ 
    DynamicJsonDocument  toJson(){
        DynamicJsonDocument doc(512);
        doc["code"] = _code;
        doc["alm"] = _alm;
        doc["timestamp"] = _timestamp;
        return doc;
    }
};

class Energia{
private:
    float _L1;
    float _L2;
    float _Load;
public:
    Status status;
    
    /*CONSTRUCTOR Class Status
    ======================================*/
    Energia(): status(),
    _L1(0.0), _L2(0.0), _Load(0.0) {}

    /*GETTER atributos privados Class Energia
    ==========================================*/
    float L1() const{
        return _L1;
    }
    float L2() const{
        return _L2;
    }
    float Load() const{
        return _Load;
    }
    
    /*SETTER atributos privados Class Energia
    ==========================================*/
    void setL1(float L1) {
        _L1 = L1;
    }
    void setL2(float L2){
        _L2 = L2;
    }
    void setLoad(float Load){
    _Load = Load;
}

    /*ARDUINO-JSON: Convert Class Energia to JSON
    =================================================*/
    DynamicJsonDocument toJson(){
        DynamicJsonDocument doc(512);
        doc["L1"] = _L1;
        doc["L2"] = _L2;
        doc["Load"] = _Load;
        doc["status"] = status.toJson();
        //return doc.as<JsonObject>();
        return doc;
    }
};

class Battery{
private:
    int _cant;
    int _capacity;
    const char* _id;
    const char* _marca;
    const char* _model;
    float _volt;
    float _lowBatVolt;
public:
    Status status;
    
    /*CONSTRUCTOR Class Battery
    ====================================================================================*/
    Battery(int cant, int capacity): _cant(cant), _capacity(capacity), 
    status(), _id(""), _marca(""), _model(""), _volt(0.0), _lowBatVolt(LOW_BAT_VOLT) {}

    /*GETTER atributos privados Class Battery
    ==========================================*/
    int cant() const{
        return _cant;
    }
    int capacity() const{
        return _capacity;
    }
    const char* id() const{
        return _id;
    }
    const char* marca() const{
        return _marca;
    }
    const char* model() const{
        return _model;
    }
    float volt() const{
        return _volt;
    }
    float lowBatVolt() const{
        return _lowBatVolt;
    }

    /*SETTER atributos privados Class Battery
    ==========================================*/
    void setCant(int cant){
        _cant = cant;
    }
    void setCapacity(int capacity){
        _capacity = capacity;
    }
    void setId(const char* id){
        _id = id;
    }
    void setMarca(const char* marca){
        _marca = marca;
    }
    void setModel(const char* model){
        _model = model;
    }
    void setLowBatVolt(float lowVolt){
        _lowBatVolt = lowVolt;
    }

    /*ARDUINO-JSON: Convert Class Battery to JSON
    =================================================*/
    DynamicJsonDocument toJson(){
        DynamicJsonDocument doc(512);
        doc["lowBATVolt"] = _lowBatVolt;
        doc["cant"] = _cant;
        doc["capacity"] = _capacity;
        doc["status"] = status.toJson();
        return doc;
    }
};

class TTA{
private:
    const char* _id;
    const char* _model;

public:
    Status status;

    /*CONSTRUCTOR Class Battery
    =========================================*/
    TTA(): status(), _id(""), _model("") {}

    /*GETTER atributos privados Class TTA
    =========================================*/
    const char* id() const{
        return _id;
    }
    const char* model(){
        return _model;
    }

    /*SETTER atributos privados Class TTA
    =========================================*/
    void setId(const char* id){
        _id = id;
    }
    void setModel(const char* model){
        _model = model;
    }

    /*ARDUINO-JSON: Convert Class TTA to JSON
    ===========================================*/
    DynamicJsonDocument toJson(){
        DynamicJsonDocument doc(512);
        doc["status"] = status.toJson();
        return doc;
    }
};

class Combustible{
private:
    const char* _level;

public:
    Status status;

    /*CONSTRUCTOR Class Combustible
    ==============================================*/
    Combustible(): status(), _level(LOW_FUEL) {}

    /*GETTER atributos Class Combustible
    =======================================*/
    const char* level() const{
        return _level;
    }

    /*SETTER atributos Class Combustible
    =======================================*/
    void setLevel(const char* level){
        _level = level;
    }
};

class Controller{
private:
    const char* _marca;
    const char* _model;

public:
    Status status;
    
    /*CONSTRUCTOR Class Controller
    ===================================================*/
    Controller(): status(), _marca(""), _model("") {}

    /*GETTER atributos Class Controller
    =======================================*/
    const char* marca() const{
        return _marca;
    }
    const char* model() const{
        return _model;
    }

    /*SETTER atributos Class Controller
    =======================================*/
    void setMarca(const char* marca){
        _marca = marca;
    }
    void setModel(const char* model){
        _model = model;
    }
};

class Rectifier{
private:
    int _cant;
    float _capacity;
    const char* _marca;
    const char* _model;

public:
    Status status;
    
    /*CONSTRUCTOR Class Rectifier
    =======================================================================*/
    Rectifier(int cant, float capacity): _cant(cant), _capacity(capacity),
    status(), _marca(""), _model("") {}

    /*GETTER atributos Class Rectifier
    ====================================*/
    int cant() const {
    return _cant;
    }   
    float capacity() const {
    return _capacity;
    }
    const char* marca() const{
        return _marca;
    }
    const char* model() const{
        return _model;
    }

    /*SETTER atributos Class Rectifier
    ====================================*/
    void setCant(int cant){
        _cant = cant;
    }
    void setCapacity(float capacity){
        _capacity = capacity;
    }
    void setMarca(const char* marca){
        _marca = marca;
    }
    void setModel(const char* model){
        _model = model;
    }
};

class PowerSystem{
private:
    float _volt;
    float _lowDCVolt;
    float _LLVD;
    float _BLVD;
    float _capacity;
    const char* _id;
    const char* _model;        

public:
    Rectifier rectifier;
    Battery battery;
    Energia ac; 
    Status status;
    
    /*CONSTRUCTOR Class PowerSystem
    =============================================================================================*/
    PowerSystem(Energia& ac, Rectifier& rectifier, Battery& battery): 
    ac(ac), rectifier(rectifier), battery(battery),
    _volt(0.0), _lowDCVolt(LOW_DC_VOLT), _LLVD(LOAD_VOLT_DISCONNECT), _BLVD(BAT_VOLT_DISCONNECT),
    _id(""), _model(""), status()
    {
        _capacity = rectifier.cant() * rectifier.capacity();
    } 

    /*GETTER atributos Class PowerSystem
    ======================================*/
    float volt() const{
        return _volt;
    }
    float lowDCVolt() const{
        return _lowDCVolt;
    }
    float LLVD() const{
        return _LLVD;
    }
    float BLVD() const{
        return _BLVD;
    }
    float capacity() const{
        return _capacity;
    }
    const char* id() const{
        return _id;
    }
    const char* model() const{
        return _model;
    }
    
    /*SETTER atributos Class PowerSystem
    ======================================*/
    void setVolt(float volt){
        _volt = volt;
    }
    void setLowDCVolt(float lowDCVolt){
        _lowDCVolt = lowDCVolt;
    }
    void setLLVD(float LLVD){
        _LLVD = LLVD;
    }
    void setBLVD(float BLVD){
        _BLVD = BLVD;
    }
    void setCapacity(){
        _capacity = rectifier.cant() * rectifier.capacity(); 
    }
    void setId(const char* id){
        _id = id;
    }
    void setModel(const char* model){
        _model = model;
    }

    /*ARDUINO-JSON: Convert Class PowerSystem to JSON
    ==================================================*/
    DynamicJsonDocument toJson(){
        DynamicJsonDocument doc(1024);
        //doc["lowDCVolt"] = _lowDCVolt;
        doc["BAT"] = battery.toJson();
        //doc["status"] = status.toJson();
        return doc;
    }
};

class GEN{
private:
    const char* _model;
    const char* _id;
    int _KVA;
    const char* _workingMode;
    boolean _on;
    float _L1;
    float _L2;
    float _Load;
    
public:
    Energia ac;
    TTA tta;
    Combustible combustible;
    Controller controller;
    Battery battery;
    Status status;

    /*CONSTRUCTOR Class GEN
    =============================================================================================================*/
    GEN(Energia& ac, TTA& tta, Combustible& combustible, Controller& controller, Battery& battery): 
    ac(ac), tta(tta), combustible(combustible), controller(controller), battery(battery), 
    status(), _model(""), _id(""), _KVA(10), _workingMode(AUTOMATIC), _on(OFF), _L1(0.0), _L2(0.0), _Load(0.0) {}

    /*GETTER atributos Class GEN
    ======================================*/
    const char* model() const{
        return _model;
    }
    const char* id() const{
        return _id;
    }
    int KVA() const{
        return _KVA;
    }
    const char* workingMode() const{
        return _workingMode;
    }
    boolean on() const{
        return _on;
    }
    float L1() const{
        return _L1;
    }
    float L2() const{
        return _L2;
    }
    float Load() const{
        return _Load;
    }

    /*SETTER atributos Class GEN
    ======================================*/
    void setModel(const char* model){
        _model = model;
    }
    void setId(const char* id){
        _id = id;
    }
    void setKVA(int KVA) {
        _KVA = KVA;;
    }
    void setWorkingMode(const char* workingMode) {
        _workingMode = workingMode;
    }
    void setOn(boolean on){
        _on = on;
    }
    void setL1(float L1){
        _L1 = L1;
    }
    void setL2(float L2){
        _L2 = L2;
    } 
    void setLoad(float Load){
        _Load = Load;
    }

    /*ARDUINO-JSON: Convert Class GEN to JSON
    ============================================*/
    DynamicJsonDocument toJson(){
        DynamicJsonDocument doc(1024);
        doc["on"] = _on;
        doc["status"] = status.toJson();
        doc["TTA"] = tta.toJson();
        doc["AC"] = ac.toJson();
        return doc;
    }
};

class RBS{
    private:
        const char* _id;
        const char* _name;
        
    public:
        Energia energy;
        GEN gen;
        PowerSystem powerSystem;
        Status status;
        
        /*CONSTRUCTOR Class RBS
        ==========================================================*/
        RBS(Energia& energy, GEN& gen, PowerSystem& powerSystem): 
        energy(energy), gen(gen), powerSystem(powerSystem), 
        _id(""), _name(""), status() {}

        /*GETTER atributos Class RBS
        ==============================*/
        const char* id() const{
            return _id;
        }
        const char* name() const{
            return _name;
        }

        /*SETTER atributos Class RBS
        ==============================*/
        void setId(const char* id){
            _id = id;
        }

        void setName(const char* name){
            _name = name;
        }

        /*ARDUINO-JSON: Convert Class RBS to JSON
        ==================================================*/
        DynamicJsonDocument toJson(){
            DynamicJsonDocument doc(2048);
            JsonObject obj = doc.createNestedObject("RBS");
            obj["name"] = _name;
            obj["GEN"] = gen.toJson();
            obj["powerSystem"] = powerSystem.toJson();
            return doc;
        }
};