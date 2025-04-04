#ifdef ESP32
#include "CommsManager.h"

#include <globalstate.h>


WiFiClient client;


void HttpEvent(HttpEvent_t *event) {
    GlobalState *gs =  GlobalState::getInstance();
    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            if(!strcmp(event->header_key, "Content-Length")){
                int length;
                char * pEnd;
                length = strtol(event->header_value, &pEnd, 10);
                gs->setOTAContentLength(length);
            }
            break;
        case HTTP_EVENT_ON_DATA:
            gs->setOTABytes(gs->getOTABytes() + event->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
}

CommsManager::CommsManager(/* args */) {}

CommsManager::~CommsManager() {}

void CommsManager::wifiConnect() {

    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }

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
            ESP.restart();
        } else if (otastatus == HTTPS_OTA_FAIL) {
        }
    }
}

void CommsManager::wifi_loop() {
    //DEBUG_F("wLOOP");
    if (WiFi.status() == WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }
    wifiConnect();
}


void CommsManager::setup() {

    String mac = WiFi.macAddress();
    mac.replace(":", "");
}
#endif