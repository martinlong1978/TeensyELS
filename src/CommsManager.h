#ifndef CommsManager_h
#define CommsManager_h
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include "config.h"
#include "SECRETS.h"
#include <HTTPClient.h>
#include <HttpsOTAUpdate.h>


#define CHECK_SUFFIX(subject, suffix) strcmp(&(subject[strlen(subject) - sizeof(suffix) + 1]), suffix) == 0

class CommsManager 
{
private:
    const char *server_certificate = "";
    bool updating = false;
    char *stateSubject;
    char *switchStateSubject;
    char *statePercentSubject;
    char *attributeSubject;
    const char *openedBy;
    int pct;
    bool state;
    bool switchState;
    int switchRead;
    void wifiConnect();
    void wifi_loop();
    HttpsOTAStatus_t otastatus;
    int relayConfig[4];


public:
    CommsManager(/* args */);
    ~CommsManager();
    void setup();
    void loop();
};



#endif