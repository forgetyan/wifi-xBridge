/*
 * WebServer.c - Library for managing a webserver that will allow user to configure the Wifi xBridge
 */

#include "WebServer.h"

ESP8266WebServer WebServer::_webServer(80);
Configuration WebServer::_configuration;
DexcomHelper WebServer::_dexcomHelper;
/*
 * Constructor
 */
WebServer::WebServer(){
  WebServer::ACCESS_POINT_SSID = "wifi-xBridge";
  WebServer::ACCESS_POINT_PWD = "";
}

/*
 * WebServer::padding
 * ------------------
 * This method is used to pad a number with '0' to make sure it is a specific length
 * number: The number to pad with '0'
 * width: The length the number should have
 */
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

/*
 * WebServer::start
 * ----------------
 * This method starts the access point and webserver on the default IP (192.168.4.1)
 */
void WebServer::start(){
  Serial.print("Configuring access point...\r\n");
  WiFi.softAP(WebServer::ACCESS_POINT_SSID, WebServer::ACCESS_POINT_PWD);
  Serial.print("SSID: ");
  Serial.print(WebServer::ACCESS_POINT_SSID);
  Serial.print("\r\nPWD: ");
  Serial.print(WebServer::ACCESS_POINT_PWD);
  Serial.print("\r\n");
  IPAddress myIP = WiFi.softAPIP();
  WebServer::_webServer.on("/", std::bind(&WebServer::handleRoot, this));
  WebServer::_webServer.on("/Test", std::bind(&WebServer::handleTest, this));
  WebServer::_webServer.on("/savetransmitterid", std::bind(&WebServer::handleSaveTransmitterId, this));
  WebServer::_webServer.on("/saveappengineaddress", std::bind(&WebServer::handleSaveAppEngineAddress, this));
  WebServer::_webServer.on("/savessid", std::bind(&WebServer::handleSaveSSID, this));
  WebServer::_webServer.on("/scanwifi", std::bind(&WebServer::handleScanWifi, this));
  WebServer::_webServer.on("/style.css", std::bind(&WebServer::handleStylesheet, this));
  WebServer::_webServer.on("/script.js", std::bind(&WebServer::handleJavascript, this));
  //WebServer::_webServer.onNotFound(std::bind(&WebServer::handleNotFound, this));
  WebServer::_webServer.begin();
}

/*
 * WebServer::loop
 * ---------------
 * This method is called by the main program at each "loop" call. Should handle all web requests
 */
void WebServer::loop() {
  WebServer::_webServer.handleClient();
}

/*
 * WebServer::getDexcomId
 * ----------------------
 * This method will get the Transmitter in EEPROM and return it
 * returns: Dexcom Transmitter Id
 */
String WebServer::getDexcomId() {
  uint32_t transmitterId = WebServer::_configuration.getTransmitterId();
  char textNbChar [5];
  _dexcomHelper.IntToCharArray(transmitterId, textNbChar);
  return WebServer::_dexcomHelper.DexcomSrcToAscii(transmitterId);
}

/*
 * WebServer::redirect
 * -------------------
 * This method will send a redirect to the client
 * url: Url to redirect to
 */
void WebServer::redirect(String url) {
  String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: " + url + "\r\nCache-Control: no-cache\r\n\r\n";
  WebServer::_webServer.sendContent(header);
}

/*
 * handleSaveTransmitterId
 * -----------------------
 * This page will save the provided Transmitter ID to EEPROM
 */
void WebServer::handleSaveTransmitterId() {
  uint32_t transmitterIdSource;
  if (WebServer::_webServer.hasArg("TransmitterId")) {
    String transmitterId = WebServer::_webServer.arg("TransmitterId");
    char transmitterCharList[6];
    transmitterId.toCharArray(transmitterCharList, 6);
    transmitterIdSource = WebServer::_dexcomHelper.DexcomAsciiToSrc((char*)transmitterCharList);
    WebServer::_configuration.setTransmitterId(transmitterIdSource);
    WebServer::_configuration.SaveConfig();
  }
  //char textNbChar [5];
  //_dexcomHelper.IntToCharArray(transmitterIdSource, textNbChar);
  WebServer::redirect("/?TransmitterSaved=1");// + String(textNbChar));
}

/*
 * WebServer::handleSaveSSID
 * -------------------------
 * This page will save the SSID / password pair in EEPROM
 */
void WebServer::handleSaveSSID() {
  if (WebServer::_webServer.hasArg("ssid_name")) {
    String ssidName = WebServer::_webServer.arg("ssid_name");
    String ssidPassword = WebServer::_webServer.arg("ssid_password");
    WebServer::_configuration.saveSSID(ssidName, ssidPassword);
    WebServer::_configuration.SaveConfig();
  }
  WebServer::redirect("/?AppEngineSaved=1");
}

/*
 * WebServer::handleSaveAppEngineAddress
 * -------------------------------------
 * This page will save the app engine address specified
 */
void WebServer::handleSaveAppEngineAddress() {
  if (WebServer::_webServer.hasArg("Address")) {
    String address = WebServer::_webServer.arg("Address");
    WebServer::_configuration.setAppEngineAddress(address);
    WebServer::_configuration.SaveConfig();
  }
  WebServer::redirect("/?AppEngineSaved=1");
}
/*
 * WebServer::handleNotFound
 * -------------------------
 * This method will filter page not found
 * If /test/[SSID] is called, it will try to connect to the server and report success or failure
 */
/*void WebServer::handleNotFound() {
  if (WebServer::_webServer.uri().startsWith("/test")) {
    // Answer to test calls
    WebServer::handleTest(WebServer::_webServer.uri().substring(6));
  }
  else {
    // 404
    WebServer::_webServer.send(404, "text/html", "Sorry, " + WebServer::_webServer.uri() + " was not found");
  }
}*/


/*
 * WebServer::handleTest
 * ---------------------
 * This page will handle the test wifi connection
 */
void WebServer::handleTest() { 
  String response = "Connection Failed";
  
  WebServer::_webServer.send(200, "text/html", response);
}

/*
 * WebServer::handleScanWifi
 * -------------------------
 * This web method will return result of a Wifi Scan
 */
void WebServer::handleScanWifi() {
  String response;
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    response = "No network found...";
  }
  else
  {
    response = "<table>\n\
 <tr>\n\
    <th align=\"left\">SSID</th>\n\
    <th></th>\n\
    <th></th>\n\
  </tr>\n";
  for (int i = 0; i < n; ++i)
  {
    String textSecurity = "";
    switch(WiFi.encryptionType(i))
    {
      case ENC_TYPE_WEP:
      case ENC_TYPE_TKIP:
      case ENC_TYPE_CCMP:
      case ENC_TYPE_AUTO:
        textSecurity = "*";
        break;
      case ENC_TYPE_NONE:
        textSecurity = "";
        break;
    }
    int rssi = WiFi.RSSI(i);
    String barClass = "";
    if(rssi > -60)
    {
      barClass = "good five-bars";
    }
    else if (rssi > -70)
    {
      barClass = "good four-bars";
    }
    else if (rssi > -80)
    {
      barClass = "ok three-bars";
    }
    else if (rssi > -90)
    {
      barClass = "bad two-bars";
    }
    else
    {
      barClass = "bad one-bar";
    }
    
    response += "  <tr>\n\
    <td>" + WiFi.SSID(i) + textSecurity + "</td>\n\
    <td>\n\
      <div class=\"signal-bars mt1 sizing-box " + barClass + "\">\n\
        <div class=\"first-bar bar\"></div>\n\
        <div class=\"second-bar bar\"></div>\n\
        <div class=\"third-bar bar\"></div>\n\
        <div class=\"fourth-bar bar\"></div>\n\
        <div class=\"fifth-bar bar\"></div>\n\
      </div>\n\
    </td>\n\
    <td align=\"right\"><a href=\"javascript:OpenSSIDPopup('Drake');\" class=\"button\">Add</a></td>\n\
  </tr>\n";
  }
  /*<tr>\n\
    <td>Drake (S)</td>\n\
    <td>\n\
      <div class=\"signal-bars mt1 sizing-box good four-bars\">\n\
        <div class=\"first-bar bar\"></div>\n\
        <div class=\"second-bar bar\"></div>\n\
        <div class=\"third-bar bar\"></div>\n\
        <div class=\"fourth-bar bar\"></div>\n\
        <div class=\"fifth-bar bar\"></div>\n\
      </div>\n\
    </td>\n\
    <td align=\"right\"><a href=\"javascript:OpenSSIDPopup('Drake');\" class=\"button\">Add</a></td>\n\
  </tr>\n\
  <tr>\n\
    <td>Monique</td>\n\
    <td>\n\
      <div class=\"signal-bars mt1 sizing-box bad one-bar\">\n\
        <div class=\"first-bar bar\"></div>\n\
        <div class=\"second-bar bar\"></div>\n\
        <div class=\"third-bar bar\"></div>\n\
        <div class=\"fourth-bar bar\"></div>\n\
        <div class=\"fifth-bar bar\"></div>\n\
      </div>\n\
    </td>\n\
    <td align=\"right\"><a href=\"javascript:OpenSSIDPopup('Monique');\" class=\"button\">Add</a></td>\n\
  </tr>\n\*/
    response += "</table>";
  }
  WebServer::_webServer.send(200, "text/html", response);
}

/*
 * WebServer::handleRoot
 * ---------------------
 * This method handle a request to the root '/' webpage
 */
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

  String configuredWifiText;
  Serial.print("Get Wifi Count");
  int wifiCount = WebServer::_configuration.getWifiCount();
  /*if (wifiCount > 0) {
    configuredWifiText = "<table>\n\
        <tr>\n\
          <th align=\"left\">SSID</th>\n\
          <th></th>\n\
        </tr>\n";
    for(int i = 0; i < wifiCount; i++)
    {
      Serial.print("Get wifi");
      Serial.print(i);
      WifiData wifiData = WebServer::_configuration.getWifiData(i);
      configuredWifiText = configuredWifiText + "<tr>\n\
          <td>" + wifiData.ssid + " (Connected)</td>\n\
          <td align=\"right\">\n\
            <a href=\"javascript:TestSSID('Drake'); \" class=\"button\">Test</a>\n\
            <a href=\"javascript:RemoveSSID('Drake');\" class=\"button\">Delete</a>\n\
          </td>\n\
        </tr>\n";
    }
    configuredWifiText = configuredWifiText + "</table>\n";
    Serial.print("Done listing wifi");
  }
  else {
    configuredWifiText = "No Wifi configured";
  }*/
  
  String response = "<html>\n\
 <head>\n\
    <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\n\
    <script src=\"script.js\"></script>\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /> \n\
    <title>wifi-xBridge Configuration Page</title>\n\
  </head>\n\
  <body>\n\
    <div id=\"popup\" class=\"overlay\">\n\
      <div class=\"popup\">\n\
        <form method=\"post\" action=\"savessid\" id=\"frmSaveSSID\">\n\
          <h2>Add a new SSID</h2>\n\
          <a class=\"close\" href=\"javascript:ClosePopup();\">&times;</a>\n\
          <div class=\"content\">\n\
            <p>Please enter the password for this wifi</p>\n\
            <input type=\"hidden\" id=\"ssid_name\"/>\n\
            <span class=\"label\" id=\"ssid_name_text\"></span>\n\
            <p><input id=\"ssid_password\" name=\"ssid_password\" type=\"password\" class=\"textbox\"/></p><br>\n\
            <p>\n\
              <div style=\"width:50%; float:left\">\n\
                <a class=\"button\" href=\"javascript:SaveSSID()\">Save</a>\n\
              </div>\n\
              <div style=\"width:50%; float:left\">\n\
                <a class=\"button\" href=\"javascript:ClosePopup();\">Close</a>\n\
              </div>\n\
            </p>\n\
            <br><br>\n\
          </div>\n\
        </form>\n\
      </div>\n\
    </div>\n\
    <h1>wifi-xBridge Configuration Page</h1>\n\
    <div class=\"innerPage\">\n\
      <h2 class=\"first\">Uptime</h2>\n\
      " + uptime + "\n\
      <h2>Dexcom ID</h2>\n\
      <p>\n\
      <input type=\"text\" id=\"txtTransmitterId\" class=\"textbox\" value=\"" + WebServer::getDexcomId() + "\">\n\
      </p>\n\
      <p>\n\
      <a href=\"javascript:SaveTransmitterId();\" class=\"button\">Save</a><br/><br/>\n\
      </p>\n\
      <h2>Google App Engine Address</h2>\n\
      <p>\n\
      <input type=\"text\" id=\"txtAppEngineAddress\" class=\"textbox\" value=\"" + WebServer::_configuration.getAppEngineAddress() + "\">\n\
      </p>\n\
      <p>\n\
      <a href=\"javascript:SaveAppEngineAddress();\" class=\"button\">Save</a><br/><br/>\n\
      </p>\n\
      <h2>Configured Wifi</h2>\n\
      <table>\n\
        <tr>\n\
          <th align=\"left\">SSID</th>\n\
          <th></th>\n\
        </tr>\n\
        <tr>\n\
          <td>Drake (Connected)</td>\n\
          <td align=\"right\">\n\
            <a href=\"javascript:TestSSID('Drake'); \" class=\"button\">Test</a>\n\
            <a href=\"javascript:RemoveSSID('Drake');\" class=\"button\">Delete</a>\n\
          </td>\n\
        </tr>\n\
        <tr>\n\
          <td>Monique</td>\n\
          <td align=\"right\">\n\
            <a href=\"javascript:TestSSID('Monique'); \" class=\"button\">Test</a>\n\
            <a href=\"javascript:RemoveSSID('Monique'); \" class=\"button\">Delete</a>\n\
          </td>\n\
        </tr>\n\
      </table>\n\
      \n\
      <br/><h2>Configure new Wifi</h2>\n\
        <a class=\"button\" href=\"javascript:ScanWifi()\">\n\
          <div style=\"width:20px; height:20px;display: inline-block; z-index: -1;\">\n\
            <div class=\"wifi-symbol\">\n\
              <div class=\"wifi-circle first\"></div>\n\
              <div class=\"wifi-circle second\"></div>\n\
              <div class=\"wifi-circle third\"></div>\n\
              <div class=\"wifi-circle fourth\"></div>\n\
            </div>\n\
          </div>\n\
          Scan for Wifi\n\
        </a><br/><br/>\n\
        <div id=\"scannedWifi\">\n\
        </div>\n\
      <!--table>\n\
        <tr>\n\
          <th align=\"left\">SSID</th>\n\
          <th></th>\n\
          <th></th>\n\
        </tr>\n\
        <tr>\n\
          <td>Drake (S)</td>\n\
          <td>\n\
            <div class=\"signal-bars mt1 sizing-box good four-bars\">\n\
              <div class=\"first-bar bar\"></div>\n\
              <div class=\"second-bar bar\"></div>\n\
              <div class=\"third-bar bar\"></div>\n\
              <div class=\"fourth-bar bar\"></div>\n\
              <div class=\"fifth-bar bar\"></div>\n\
            </div>\n\
          </td>\n\
          <td align=\"right\"><a href=\"javascript:OpenSSIDPopup('Drake');\" class=\"button\">Add</a></td>\n\
        </tr>\n\
        <tr>\n\
          <td>Monique</td>\n\
          <td>\n\
            <div class=\"signal-bars mt1 sizing-box bad one-bar\">\n\
              <div class=\"first-bar bar\"></div>\n\
              <div class=\"second-bar bar\"></div>\n\
              <div class=\"third-bar bar\"></div>\n\
              <div class=\"fourth-bar bar\"></div>\n\
              <div class=\"fifth-bar bar\"></div>\n\
            </div>\n\
          </td>\n\
          <td align=\"right\"><a href=\"javascript:OpenSSIDPopup('Monique');\" class=\"button\">Add</a></td>\n\
        </tr>\n\
      </table-->\n\
      \n\
    </div>\n\
  </body>\n\
</html>";
  
  WebServer::_webServer.send(200, "text/html", response);
}

/*
 * WebServer::handleRoot
 * ---------------------
 * This method handle a request to the stylesheet '/style.css' webpage
 */
void WebServer::handleStylesheet() {
  String response = "";
  response += "body { font-family: Arial, sans-serif; background-color: #9EDFFF;  }h1 { color:  3377FF; margin-left: 20px;  font-size: 30px;  text-align: center; text-shadow:    -1px -1px 1px #666666,    2px 2px 1px #333333;}.innerPage{  background-color: white;  border: 1px solid black;  padding: 8px; box-shadow: 10px 10px 5px #5888C8;  //color: #9E8042; margin: 0 auto; border: 2px solid #000080;  border-radius: 10px/10px; text-align: center;}h2 {  color: #000080; }h2.First {   margin-top: 4px;}table {  border-collapse: collapse;  width:100%; color: #800000; margin-bottom: 15px}th, td {  padding-top: 15px;  padding-bottom: 15px; border-bottom: 1px solid #ddd;}.button {  font-size: 1em;  padding: 10px;  border: 2px solid #000080;  border-radius: 20px/50px;  text-decoration: none;  cursor: pointer;  transition: all 0.3s ease-out;  margin: 5px;}.button:hover {  background: #9EDFFF;}.overlay {  position: fixed;  top: 0;  bottom: 0;  left: 0;  right: 0;  background: rgba(0, 0, 0, 0.7);  transition: opacity 500ms;  visibility: hidden;  opacity: 0;  z-index: 999;}.overlay:target {  visibility: visible;  opacity: 1;}.popup {  margin: 70px auto;  padding: 20px;  background: #fff;  border-radius: 5px;  width: 30%;  position: relative;  transition: all 5s ease-in-out;  text-align: center;  }.popup h2 {  margin-top: 0;  color: #333;  font-family: Tahoma, Arial, sans-serif;  }.popup .close {  position: absolute;  top: 20px;  right: 30px;  transition: all 200ms;  font-size: 30px;  font-weight: bold;  text-decoration: none;  color: #333;}.popup .close:hover {  color: #3377FF;}.popup .content {  max-height: 90%;  overflow: auto;}@media screen and (max-width: 700px){  .popup{  width: 90%;  }}.textbox { border: 5px solid white;  -webkit-box-shadow:     inset 0 0 8px  rgba(0,0,0,0.1),     0 0 16px rgba(0,0,0,0.1);   -moz-box-shadow:    inset 0 0 8px  rgba(0,0,0,0.1),     0 0 16px rgba(0,0,0,0.1);   box-shadow:     inset 0 0 8px  rgba(0,0,0,0.1),     0 0 16px rgba(0,0,0,0.1);   padding: 15px;  background: rgba(255,255,255,0.5);  margin: 0 0 7px 0;  font-size: 20px;  width:100%;}.label {  font-size: 1.17em;  font-weight: bold;}.wifi-symbol {  display: none;}.wifi-symbol [foo], .wifi-symbol {  position: absolute;  display: inline-block;  width: 20px;  height: 20px;  margin-top: -72px;  margin-left: 60px;  -ms-transform: rotate(-45deg) translate(-100px);  -moz-transform: rotate(-45deg) translate(-100px);  -o-transform: rotate(-45deg) translate(-100px);  -webkit-transform: rotate(-45deg) translate(-100px);  transform: rotate(-45deg) translate(-100px);}.wifi-symbol .wifi-circle {  box-sizing: border-box;  -moz-box-sizing: border-box;  display: block;  width: 100%;  height: 100%;  font-size: 2.86px;  position: absolute;  bottom: 0;  left: 0;  border-color: #000055;  border-style: solid;  border-width: 1em 1em 0 0;  -webkit-border-radius: 0 100% 0 0;  border-radius: 0 100% 0 0;  opacity: 0;  -o-animation: wifianimation 3s infinite;  -moz-animation: wifianimation 3s infinite;  -webkit-animation: wifianimation 3s infinite;  animation: wifianimation 3s infinite;}.wifi-symbol .wifi-circle.first {  -o-animation-delay: 800ms;  -moz-animation-delay: 800ms;  -webkit-animation-delay: 800ms;  animation-delay: 800ms;}.wifi-symbol .wifi-circle.second {  width: 5em;  height: 5em;  -o-animation-delay: 400ms;  -moz-animation-delay: 400ms;  -webkit-animation-delay: 400ms;  animation-delay: 400ms;}.wifi-symbol .wifi-circle.third {  width: 3em;  height: 3em;}.wifi-symbol .wifi-circle.fourth {  width: 1em;  height: 1em;  opacity: 1;  background-color: #000055;  -o-animation: none;  -moz-animation: none;  -webkit-animation: none;  animation: none;}@-o-keyframes wifianimation {  0% {    opacity: 0.4;  }  5% {    opactiy: 1;  }  6% {    opactiy: 0.1;  }  100% {    opactiy: 0.1;  }}@-moz-keyframes wifianimation {  0% {    opacity: 0.4;  }  5% {    opactiy: 1;  }  6% {    opactiy: 0.1;  }  100% {    opactiy: 0.1;  }}@-webkit-keyframes wifianimation {  0% {    opacity: 0.4;  }  5% {    opactiy: 1;  }  6% {    opactiy: 0.1;  }  100% {    opactiy: 0.1;  }}* {  box-sizing: border-box;}.sizing-box {  height: 20px;  width: 80px;}.signal-bars {  display: inline-block;}.signal-bars .bar {  width: 14%;  margin-left: 0%;  min-height: 20%;  display: inline-block;}.signal-bars .bar.first-bar {  height: 20%;}.signal-bars .bar.second-bar {  height: 40%;}.signal-bars .bar.third-bar {  height: 60%;}.signal-bars .bar.fourth-bar {  height: 80%;}.signal-bars .bar.fifth-bar {  height: 99%;}.good .bar {  background-color: #16a085;  border: thin solid #12816b;}.bad .bar {  background-color: #e74c3c;  border: thin solid #a82315;}.ok .bar {  background-color: #f1c40f;  border: thin solid #d0a90c;}.four-bars .bar.fifth-bar,.three-bars .bar.fifth-bar,.three-bars .bar.fourth-bar,.one-bar .bar:not(.first-bar),.two-bars .bar:not(.first-bar):not(.second-bar) {  background-color: #fafafa;  border: thin solid #f3f3f3;}";
  WebServer::_webServer.send(200, "text/css", response);
}

/*
 * WebServer::handleRoot
 * ---------------------
 * This method handle a request to the javascript '/script.js' webpage
 */
void WebServer::handleJavascript() {
  String response = "function OpenSSIDPopup(ssid)\n\
{\n\
 var popup = document.getElementById(\"popup\");\n\
  var ssid_name = document.getElementById(\"ssid_name\");\n\
  var ssid_name_text = document.getElementById(\"ssid_name_text\");\n\
  var ssid_password = document.getElementById(\"ssid_password\");\n\
  ssid_name.value = ssid;\n\
  ssid_name_text.innerHTML = ssid;\n\
  ssid_password.value = \"\";\n\
  popup.style.visibility = \"visible\";\n\
  popup.style.opacity = 1;\n\
}\n\
\n\
function SaveTransmitterId() {\n\
  document.location.href='/savetransmitterid?TransmitterId=' + document.getElementById(\"txtTransmitterId\").value;\n\
}\n\
\n\
function SaveAppEngineAddress() {\n\
  document.location.href='/saveappengineaddress?Address=' + document.getElementById(\"txtAppEngineAddress\").value;\n\
}\n\
\n\
function SaveSSID() {\n\
  var ssid_name = document.getElementById(\"ssid_name\");\n\
  var ssid_password = document.getElementById(\"ssid_password\");\n\
  var frmSaveSSID = document.getElementById(\"frmSaveSSID\");\n\
  frmSaveSSID.submit();\n\
}\n\
\n\
function ClosePopup() {\n\
  var popup = document.getElementById(\"popup\");\n\
  popup.style.visibility = \"hidden\";\n\
  popup.style.opacity = 0;\n\
}\n\
\n\
function RemoveSSID(ssid)\n\
{\n\
  if (confirm(\"Do you really want to remove \" + ssid)) {\n\
    document.location.href='remove/' + ssid;\n\
  }\n\
}\n\
\n\
function TestSSID(ssid) {\n\
  var xhttp = new XMLHttpRequest();\n\
  xhttp.open(\"GET\", \"test/\" + ssid, true);\n\
  xhttp.onreadystatechange = function () {\n\
    if(xhttp.readyState === XMLHttpRequest.DONE && xhttp.status === 200){\n\
      alert(xhttp.responseText);\n\
      console.log(xhttp.responseText);\n\
    };\n\
  };\n\
  xhttp.send();\n\
}\n\
\n\
\n\
function ScanWifi() {\n\
  var xhttp = new XMLHttpRequest();\n\
  xhttp.open(\"GET\", \"scanwifi\", true);\n\
  xhttp.onreadystatechange = function () {\n\
    if(xhttp.readyState === XMLHttpRequest.DONE && xhttp.status === 200){\n\
      var divScannedWifi = document.getElementById(\"scannedWifi\");\n\
      divScannedWifi.innerHTML = xhttp.responseText;\n\
    };\n\
  };\n\
  xhttp.send();\n\
  \n\
  \n\
}";
  WebServer::_webServer.send(200, "text/javascript", response);
}



