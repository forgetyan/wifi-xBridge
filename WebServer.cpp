/*
 * WebServer.c - Library for managing a webserver that will allow user to configure the Wifi xBridge
 */

#include "WebServer.h"

//const char* ACCESS_POINT_SSID = "wifi-xBridge";
//const char* ACCESS_POINT_PWD = "123456";

ESP8266WebServer _webServer(80);
/*
 * Constructor
 */
WebServer::WebServer(){
  WebServer::ACCESS_POINT_SSID = "wifi-xBridge";
  WebServer::ACCESS_POINT_PWD = "";
}

String WebServer::padding( int number, byte width ) {
  String response = "";
  int currentMax = 10;
  int currentChar = 0;
  for (byte i=1; i<width; i++){
    if (number < currentMax) {
      response += "0";
      currentChar++;
    }
    currentMax *= 10;
  } 
  response += number;
  return response;    
}

void WebServer::handleRoot() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  String uptime = "";
  uptime += WebServer::padding(hr, 2);
  uptime += ":";
  uptime += WebServer::padding(min % 60, 2);
  uptime += ":";
  uptime += WebServer::padding(sec % 60, 2);
  
  String response = "";
  response += "<html>";
  response += "<header>";
  response += "<style>";
  response += "body {";
  response += "background-color: lightblue;";
  response += "}";
  response += "h1 {";
  response += "color: navy;";
  response += "margin-left: 20px;";
  response += "font-size: 30px;";
  response += "text-align: center;";
  response += "}";
  response += "h2.First {";
  response += "margin-top: 4px;";
  response += "}";
  response += ".innerPage{";
  response += "background-color: white;";
  response += "border: 1px solid black;";
  response += "padding: 5px;";
  response += "box-shadow: 10px 10px 5px #888888;";
  response += "}";
  response += "table {";
  response += "border-collapse: collapse;";
  response += "width:100%;";
  response += "}";
  response += "table {";
  response += "//border: 1px solid black;";
  response += "}";
  response += "th, td {";
  response += "padding: 15px;";
  response += "text-align: left;";
  response += "}";
  response += "th, td {";
  response += "border-bottom: 1px solid #ddd;";
  response += "}";
  response += "</style>";
  response += "</header>";
  response += "<body>";
  response += "<h1>wifi-xBridge Configuration Page</h1>";
    
  response += "<div class=\"innerPage\">";
  response += "<h2 class=\"first\">Uptime</h2>";
  response += uptime;
  response += "<h2>Configured Wifi</h2>";
  response += "<table>";
  response += "<tr>";
  response += "<th>SSID</th>";
  response += "<th></th>";
  response += "</tr>";
  response += "<tr>";
  response += "<td>Drake</td>";
  response += "<td><a href="">Delete</a></td>";
  response += "</tr>";
  response += "</table>";
  response += "</div>";
  response += "</body>";
  response += "</html>";
/*  response += "Uptime: ";
  response += WebServer::padding(hr, 2);
  response += ":";
  response += WebServer::padding(min % 60, 2);
  response += ":";
  response += WebServer::padding(sec % 60, 2);*/
  _webServer.send(200, "text/html", response);
}

void WebServer::start(){
  Serial.print("Configuring access point...\r\n");
  WiFi.softAP(WebServer::ACCESS_POINT_SSID, WebServer::ACCESS_POINT_PWD);
  Serial.print("SSID: ");
  Serial.print(WebServer::ACCESS_POINT_SSID);
  Serial.print("\r\nPWD: ");
  Serial.print(WebServer::ACCESS_POINT_PWD);
  Serial.print("\r\n");
  IPAddress myIP = WiFi.softAPIP();
  _webServer.on("/", std::bind(&WebServer::handleRoot, this));
  _webServer.begin();
}

void WebServer::loop() {
  _webServer.handleClient();
}

