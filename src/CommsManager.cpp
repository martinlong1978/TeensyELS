#ifdef ESP32
#include "CommsManager.h"
#include "telnet.h"

#include <globalstate.h>


WiFiClient client;


void HttpEvent(HttpEvent_t *event) {
    GlobalState *gs =  GlobalState::getInstance();
    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            DEBUG_F("Http Event Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            DEBUG_F("Http Event On Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            DEBUG_F("Http Event Header Sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            DEBUG_F("Http Event On Header, key=%s, value=%s\n",
                   event->header_key, event->header_value);
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
            DEBUG_F("Http Event On Finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            DEBUG_F("Http Event Disconnected");
            break;
    }
}

CommsManager::CommsManager(/* args */) {}

CommsManager::~CommsManager() {}

void CommsManager::wifiConnect() {

    DEBUG_F("WIFI: Waiting for WiFi... ");
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
        DEBUG_F(".");
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }

    DEBUG_F("\nWIFI: WiFi connected");
    DEBUG_F("WIFI: IP address: ");
    telnet.begin();
    telnet.setNewlineCharacter('\r');
}

void CommsManager::loop() {
    wifiConnect();
    for (;;) {
        wifi_loop();
        telnet.loop();
        if(GlobalState::getInstance()->hasOTA())
        {
            GlobalState::getInstance()->clearOTA();
            HttpsOTA.onHttpEvent(HttpEvent);
            HttpsOTA.begin(UPDATE_URL, "");
        }
        otastatus = HttpsOTA.status();
        if (otastatus == HTTPS_OTA_SUCCESS) {
            DEBUG_F("Firmware written successfully.");
            ESP.restart();
        } else if (otastatus == HTTPS_OTA_FAIL) {
            DEBUG_F("Firmware Upgrade Fail");
        }
    }
}

void CommsManager::wifi_loop() {
    //DEBUG_F("wLOOP");
    if (WiFi.status() == WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }
    DEBUG_F("WIFI: Wifi is not connecting... Try to connect");
    wifiConnect();
}


void CommsManager::setup() {

    String mac = WiFi.macAddress();
    mac.replace(":", "");
}
#endif