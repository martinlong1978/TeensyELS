#ifdef ESP32
#ifndef ESPCommsManager_h
#define ESPCommsManager_h
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include "config.h"
#include <HTTPClient.h>
#include <HttpsOTAUpdate.h>
#include "WebSettings.h"



#define CHECK_SUFFIX(subject, suffix) strcmp(&(subject[strlen(subject) - sizeof(suffix) + 1]), suffix) == 0

class ESPCommsManager {
private:
    const char* server_certificate = "";
    bool updating = false;
    char* stateSubject;
    char* switchStateSubject;
    char* statePercentSubject;
    char* attributeSubject;
    const char* openedBy;
    int pct;
    bool state;
    bool switchState;
    int switchRead;
    void wifiConnect(WebSettings *webSettings);
    void wifi_loop(WebSettings *webSettings);
    HttpsOTAStatus_t otastatus;
    int relayConfig[4];


public:
    ESPCommsManager(/* args */);
    ~ESPCommsManager();
    void setup();
    void loop();
};



#endif
#endif