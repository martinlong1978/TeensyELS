#include "WebSettings.h"
#include "WebServer.h"

WebServer* webServer;


const uint32_t NVM_Offset = 0x9000;
uint32_t address = 0x3000;



void showPage() {
    Serial.println("Serving page");
    // Display the HTML web page
    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<link rel=\"icon\" href=\"data:,\">";
    html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
    html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
    html += ".button2 { background-color: #555555; }</style></head>";
    html += "<body><h1>ESP32 Web Server</h1>";
    html += "<form method='POST' action='/set'>";
    html += "<label for='ssid'>SSID:</label><br>";
    html += "<input type='text' id='ssid' name='ssid' value=''><br><br>";
    html += "<label for='password'>Password:</label><br>";
    html += "<input type='text' id='password' name='password' value=''><br><br>";
    html += "<label for='url'>Update URL:</label><br>";
    html += "<input type='text' id='url' name='url' value='http://'><br><br>";
    html += "<input type='submit' value='Submit'>";
    html += "</form>";
    html += "</body></html>";

    webServer->send(200, "text/html", html);
}

WebSettings* getWebSettings(){
    WebSettings * settings = new WebSettings();
    ESP.flashRead(NVM_Offset + address, (uint32_t *)settings, sizeof(WebSettings));
    return settings;
}

void setValues() {
    Serial.printf("SSID %s\n", webServer->arg("ssid").c_str());
    Serial.printf("Password %s\n", webServer->arg("password").c_str());
    Serial.printf("update url %s\n", webServer->arg("url").c_str());
    webServer->send(200, "text/plain", "");
    WebSettings settings;
    strcpy(settings.ssid, webServer->arg("ssid").c_str());
    strcpy(settings.password, webServer->arg("password").c_str());
    strcpy(settings.url, webServer->arg("url").c_str());

    ESP.flashEraseSector((NVM_Offset + address) / 4096);
    ESP.flashWrite(NVM_Offset + address,  (uint32_t *)&settings, sizeof(WebSettings));

}

void startWebServer() {
    webServer = new WebServer(80);
    webServer->on("/", HTTP_GET, showPage);
    webServer->on("/set", HTTP_POST, setValues);
    webServer->begin();
    Serial.println("Server is running");

}

void wifiLoop() {
    webServer->handleClient();
}

