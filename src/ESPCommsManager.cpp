#include "ESPCommsManager.h"
#include <globalstate.h>


WiFiClient client;


void HttpEvent(HttpEvent_t* event) {
    GlobalState* gs = GlobalState::getInstance();
    switch (event->event_id) {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        if (!strcmp(event->header_key, "Content-Length")) {
            int length;
            char* pEnd;
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

ESPCommsManager::ESPCommsManager(/* args */) {}

ESPCommsManager::~ESPCommsManager() {}

void ESPCommsManager::wifiConnect(WebSettings * webSettings) {

    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(webSettings->ssid, webSettings->password);
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }

}

void ESPCommsManager::loop() {
    WebSettings* webSettings = getWebSettings();
    wifiConnect(webSettings);
    for (;;) {
        wifi_loop(webSettings);
        if (GlobalState::getInstance()->hasOTA() && !updating) {
            updating = true;
            Serial.println("Beginning update");
            HttpsOTA.onHttpEvent(HttpEvent);
            HttpsOTA.begin(webSettings->url, "");
        }
        otastatus = HttpsOTA.status();
        if (otastatus == HTTPS_OTA_SUCCESS) {
            ESP.restart();
        } else if (otastatus == HTTPS_OTA_FAIL) {
        }
    }
}

void ESPCommsManager::wifi_loop(WebSettings *webSettings) {
    //DEBUG_F("wLOOP");
    if (WiFi.status() == WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }
    wifiConnect(webSettings);
}

