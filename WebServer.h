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
    void setConfiguration(Configuration configuration);
  private:
    String getDexcomId();
    void handleRoot();
    //void handleNotFound();
    void handleStylesheet();
    void handleJavascript();
    void handleScanWifi();
    void handleTest();
    void handleSaveTransmitterId();
    void handleSaveDebugConfig();
    void handleSaveHotSpotConfig();
    void handleSaveSSID();
    void handleRemoveSSID();
    void handleSaveAppEngineAddress();
    void redirect(String url);
    String padding(int number, byte width);
    static ESP8266WebServer _webServer;
    static Configuration _configuration;
    static DexcomHelper _dexcomHelper;
    void StartAccessPoint();
    
};

#endif
