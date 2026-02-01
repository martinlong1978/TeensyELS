#include "WebSettings.h"
#include "WebServer.h"

WebServer* webServer;


const uint32_t NVM_Offset = 0x9000;
uint32_t address = 0x3000;
uint32_t latheaddress = 0x3000 + sizeof(WebSettings);

#define DEFAULTWEBSETTING(setting, default) html += ((webSettings->check == CHECKVALUE ) ? setting : default)
#define DEFAULTLATHESETTING(setting, default) html += ((latheConfig->check == CHECKVALUE ) ? setting : default)

#define DEFAULTLATHEINTSETTING(setting, default, format) \
    sprintf(tempbuffer, format, setting); \
    html += ((latheConfig->check == CHECKVALUE ) ? tempbuffer : default)

#define SETCONFIG(websetting, configsetting)    configsetting = std::stoi(webServer->arg(websetting).c_str());





void showPage() {
    WebSettings* webSettings = getWebSettings();
    LatheConfig* latheConfig = getLatheSettings();
    Serial.println("Serving page");
    // Display the HTML web page

    char tempbuffer[50];

    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<link rel=\"icon\" href=\"data:,\">";
    html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
    html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
    html += ".button2 { background-color: #555555; }</style></head>";
    html += "<body><h1>ESP32 ELS - Electronic Leadscrew</h1>";
    html += "<form method='POST' action='/set'>";

    html += "<label for='ssid'>SSID: </label><br>";
    html += "<input type='text' id='ssid' name='ssid' value='";
    DEFAULTWEBSETTING(webSettings->ssid, "");
    html += "'><br><br>";

    html += "<label for='password'>Password:</label><br>";
    html += "<input type='text' id='password' name='password' value='";
    DEFAULTWEBSETTING(webSettings->password, "");
    html += "'><br><br>";

    html += "<label for='url'>Update URL:</label><br>";
    html += "<input type='text' id='url' name='url' value='";
    DEFAULTWEBSETTING(webSettings->url, "http://hass.longhome.co.uk/els/elstft.bin");
    html += "'><br><br>";

    html += "<label for='spindleEncoderPpr'>Encoder PPR:</label><br>";
    html += "<input type='number' id='spindleEncoderPpr' name='spindleEncoderPpr' value='";
    DEFAULTLATHEINTSETTING(latheConfig->spindleEncoderPpr, "1200", "%d");
    html += "'><br><br>";

    html += "<label for='stepperPpr'>Stepper PPR:</label><br>";
    html += "<input type='number' id='stepperPpr' name='stepperPpr' value='";
    DEFAULTLATHEINTSETTING(latheConfig->stepperPpr, "400", "%d");
    html += "'><br><br>";

    html += "<label for='invertDirection'>Invert motor direction:</label><br>";
    html += "<input type='checkbox' id='invertDirection' name='invertDirection' value='invert'";
    if (latheConfig->check != CHECKVALUE || latheConfig->invertDirection) {
        html += "checked";
    }
    html += "><br><br>";

    html += "<label for='gearboxRatioNumerator'>Gearbox ratio numerator:</label><br>";
    html += "<input type='number' id='gearboxRatioNumerator' name='gearboxRatioNumerator' value='";
    DEFAULTLATHEINTSETTING(latheConfig->gearboxRatioNumerator, "2", "%d");
    html += "'><br><br>";

    html += "<label for='gearboxRatioDenominator'>Gearbox ratio denominator:</label><br>";
    html += "<input type='number' id='gearboxRatioDenominator' name='gearboxRatioDenominator' value='";
    DEFAULTLATHEINTSETTING(latheConfig->gearboxRatioDenominator, "1", "%d");
    html += "'><br><br>";

    html += "<label for='leadscrewPitchMm'>Leadscrew pitch:</label><br>";
    html += "<input type='number' id='leadscrewPitchMm' name='leadscrewPitchMm' step=0.00002 value='";
    DEFAULTLATHEINTSETTING(latheConfig->leadscrewPitchMm, "2.54", "%f");
    html += "'> mm<br><br>";

    html += "<label for='jogSpeed'>Max jog speed:</label><br>";
    html += "<input type='number' id='jogSpeed' name='jogSpeed' value='";
    DEFAULTLATHEINTSETTING(latheConfig->jogSpeed, "40", "%d");
    html += "'> m/s<br><br>";
    
    html += "<label for='leadscrewAcceleration'>Leadscrew acceleration:</label><br>";
    html += "<input type='number' id='leadscrewAcceleration' name='leadscrewAcceleration' value='";
    DEFAULTLATHEINTSETTING(latheConfig->leadscrewAcceleration, "150", "%d");
    html += "'> m/s<sup>2</sup><br><br>";
    
    html += "<label for='leadscrewMaxSpeed'>Max leadscrew speed:</label><br>";
    html += "<input type='number' id='leadscrewMaxSpeed' name='leadscrewMaxSpeed' value='";
    DEFAULTLATHEINTSETTING(latheConfig->leadscrewMaxSpeed, "40", "%d");
    html += "'> m/s<br><br>";
    
    html += "<input type='submit' value='Submit'>";
    html += "</form>";
    html += "</body></html>";

    webServer->send(200, "text/html", html);
}

WebSettings* getWebSettings() {
    WebSettings* settings = new WebSettings();
    ESP.flashRead(NVM_Offset + address, (uint32_t*)settings, sizeof(WebSettings));
    return settings;
}

LatheConfig* getLatheSettings() {
    LatheConfig* settings = new LatheConfig();
    ESP.flashRead(NVM_Offset + latheaddress, (uint32_t*)settings, sizeof(LatheConfig));
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
    settings.check = CHECKVALUE;

    LatheConfig config;
    SETCONFIG("spindleEncoderPpr", config.spindleEncoderPpr);
    SETCONFIG("gearboxRatioDenominator", config.gearboxRatioDenominator);
    SETCONFIG("gearboxRatioNumerator", config.gearboxRatioNumerator);
    SETCONFIG("jogSpeed", config.jogSpeed);
    SETCONFIG("leadscrewAcceleration", config.leadscrewAcceleration);
    SETCONFIG("leadscrewMaxSpeed", config.leadscrewMaxSpeed);
    SETCONFIG("leadscrewPitchMm", config.leadscrewPitchMm);
    SETCONFIG("stepperPpr", config.stepperPpr);
    config.invertDirection = strcmp(webServer->arg("invertDirection").c_str(), "invert") == 0 ? true : false;
    config.leadscrewPitchMm = std::stof(webServer->arg("leadscrewPitchMm").c_str());
    config.check = CHECKVALUE;

    ESP.flashEraseSector((NVM_Offset + address) / 4096);
    ESP.flashWrite(NVM_Offset + address, (uint32_t*)&settings, sizeof(WebSettings));
    ESP.flashWrite(NVM_Offset + latheaddress,  (uint32_t*)&config, sizeof(LatheConfig));

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

