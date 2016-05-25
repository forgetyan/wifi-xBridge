#ifndef WebServer_h
#define WebServer_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Arduino.h"
#include "Configuration.h"
#include "DexcomHelper.h"



class WebServer {
  public:
    WebServer();
    void start();
    void loop();
  private:
    String getDexcomId();
    void handleRoot();
    //void handleNotFound();
    void handleStylesheet();
    void handleJavascript();
    void handleScanWifi();
    void handleTest();
    void handleSaveTransmitterId();
    void redirect(String url);
    // const char* ACCESS_POINT_SSID;
    // const char* ACCESS_POINT_PWD;
    char* ACCESS_POINT_SSID;
    char* ACCESS_POINT_PWD;
    String padding(int number, byte width);
    static ESP8266WebServer _webServer;
    static Configuration _configuration;
    static DexcomHelper _dexcomHelper;
    
};

#endif
