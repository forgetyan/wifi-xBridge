#ifndef WebServer_h
#define WebServer_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Arduino.h"



class WebServer {
  public:
    WebServer();
    void start();
    void loop();
  private:
    void handleRoot();
    void handleStylesheet();
    void handleJavascript();
    // const char* ACCESS_POINT_SSID;
    // const char* ACCESS_POINT_PWD;
    char* ACCESS_POINT_SSID;
    char* ACCESS_POINT_PWD;
    String padding(int number, byte width);
    
};

#endif
