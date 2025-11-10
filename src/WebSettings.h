

typedef struct WebSettings {
    char  ssid[32];
    char  password[63];
    char  url[512];
} WebSettings;

WebSettings* getWebSettings();


void startWebServer() ;

void wifiLoop() ;
