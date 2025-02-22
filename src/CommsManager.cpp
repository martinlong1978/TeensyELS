#include "CommsManager.h"

#include <string>
#include <globalstate.h>


WiFiClient client;


void HttpEvent(HttpEvent_t *event) {
    GlobalState *gs =  GlobalState::getInstance();
    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            DEBUGLN("Http Event Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            DEBUGLN("Http Event On Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            DEBUGLN("Http Event Header Sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            DEBUGF("Http Event On Header, key=%s, value=%s\n",
                   event->header_key, event->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            gs->setOTABytes(gs->getOTABytes() + event->data_len);
            DEBUGF(".", event->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            DEBUGLN("Http Event On Finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            DEBUGLN("Http Event Disconnected");
            break;
    }
}

CommsManager::CommsManager(/* args */) {}

CommsManager::~CommsManager() {}

void CommsManager::wifiConnect() {

    DEBUGLN("WIFI: Waiting for WiFi... ");
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
        DEBUGLN(".");
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }

    DEBUGLN("\nWIFI: WiFi connected");
    DEBUGLN("WIFI: IP address: ");
    DEBUGLN(WiFi.localIP());
}

void CommsManager::loop() {
    wifiConnect();
    for (;;) {
        wifi_loop();
        if(GlobalState::getInstance()->hasOTA())
        {
            GlobalState::getInstance()->clearOTA();
            HttpsOTA.onHttpEvent(HttpEvent);
            HttpsOTA.begin(UPDATE_URL, "");
        }
        otastatus = HttpsOTA.status();
        if (otastatus == HTTPS_OTA_SUCCESS) {
            Serial.println("Firmware written successfully.");
            ESP.restart();
        } else if (otastatus == HTTPS_OTA_FAIL) {
            Serial.println("Firmware Upgrade Fail");
        }
    }
}

void CommsManager::wifi_loop() {
    //DEBUGLN("wLOOP");
    if (WiFi.status() == WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }
    DEBUGLN("WIFI: Wifi is not connecting... Try to connect");
    wifiConnect();
}


void CommsManager::setup() {

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    //Serial.printf("Host is %s\n", host);
   // Serial.printf("Running version %s\n", VERSION);
}