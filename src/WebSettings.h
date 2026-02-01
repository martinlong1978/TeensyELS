
#include <stdint.h>
#include "latheconfig.h"
#ifndef WebSettings_h
#define WebSettings_h


typedef struct WebSettings {
    int32_t check;
    char  ssid[32];
    char  password[63];
    char  url[512];
} WebSettings;

WebSettings* getWebSettings();
LatheConfig* getLatheSettings();


void startWebServer() ;

void wifiLoop() ;

#endif