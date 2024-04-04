
/*Funcion para conectar a WiFi: Intentara conexion 5 veces (5 seg)
==================================================================*/
void connectToWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Conecting to WiFi network: %s...\n",ssid);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 5){
    Serial.printf(".");
    delay(delayTime);
    retry++;
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.printf("\nConection is SUCCESSFUL!!!\n");
    Serial.printf("SSID: %s\n",ssid);
    Serial.printf("IP: %s\n\n",WiFi.localIP().toString().c_str());
  }
  else
    Serial.printf("\nNO CONNECTED!!!\n\n");  
}

/*Funcion para re-conectar a WiFi. Un solo intento con mensaje de 
NO CONEXION cada n*DelayTime siempre y cuando aun no este conectado.
=====================================================================*/
void reConnectToWiFi() {
  current_wifi_event_time = millis();
  if(current_wifi_event_time - last_wifi_event_time > 10*delayTime && WiFi.status() != WL_CONNECTED){
    last_wifi_event_time = millis();
    Serial.println("\nWifi DISCONNECTED!!!");
    Serial.printf("Tray to Connect to Wifi network: %s!!!\n\n",ssid);
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(delayTime);
}

/*Funcion que indica el estatus de la conexion Wifi
====================================================*/
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

/*Funcion de eventos por conexion/desconexion del Wifi
=======================================================*/
void wifiEvent(WiFiEvent_t event){
  switch(event){
    case SYSTEM_EVENT_STA_DISCONNECTED:
      WiFi.removeEvent(wifiEvent);
      reConnectToWiFi();
      WiFi.onEvent(wifiEvent);
      break;
    
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.printf("\nSUCCESSFUL connection to Wifi!!!\n");
      Serial.printf("SSID: %s\n",ssid);
      Serial.printf("IP: %s\n\n",WiFi.localIP().toString().c_str());
      break;

    default:
      break;
  }
}

/*Función para conectar al broker MQTT, devuelve true si la conexion es exitosa caso contrario devuelve false
=====================================================================================================================*/
bool connectToMQTT(PubSubClient &mqtt_client, const char *mqtt_ID, const char *mqtt_user, const char *mqtt_password){
  if(!mqtt_client.connected()){
    Serial.printf("Tray connection to broker: %s!!!\n",mqtt_server);
    if(mqtt_client.connect(mqtt_ID, mqtt_user, mqtt_password)){
      Serial.printf("Connection to MQTT Broker %s is OK!!!\n",mqtt_server);
      return true;
    }
    else{
      Serial.printf("Error connection to broker MQTT: ");
      Serial.printf("%d\n\n",mqtt_client.state());
      return false;
    }
  }
  return true;
}

/*Función para re-conectar al broker MQTT, devuelve true si la conexion es exitosa caso contrario devuelve false
=======================================================================================================================*/
bool reConnectToMQTT(PubSubClient &mqtt_client, const char *mqtt_ID, const char *mqtt_user, const char *mqtt_password){
  current_mqtt_event_time = millis();
  if(!mqtt_client.connected() && current_mqtt_event_time - last_mqtt_event_time > 10*delayTime){
    last_mqtt_event_time = millis();
    Serial.println("\nMQTT Connection is lost!!!");
    Serial.printf("Tray connection to broker: %s!!!\n",mqtt_server);
    if(mqtt_client.connect(mqtt_ID, mqtt_user, mqtt_password)){
      Serial.printf("Connection to MQTT Broker %s is OK!!!\n\n",mqtt_server);
      return true;
    }
    else{
      Serial.printf("Error connection to broker MQTT: ");
      Serial.printf("%d\n\n",mqtt_client.state());
      return false;
    }
  }
  return true;
}