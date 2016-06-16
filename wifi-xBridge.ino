/*
 * Project URL:
 * https://github.com/irDrake/wifi-xBridge
 * 
 * ---------------------------------------------
 * 
 * This software take the place of the "Bluetooth" portion of the xBridge2 system and
 * send all data by wifi to an AppEngine server like Parakeet does.
 * 
 * The Wixel won't even tell the difference between communicating with the HM-10 
 * Bluetooth module or this code which allow "Wifi" transmission.
 * 
 * This project allow the user to send the data wirelessly without the need to carry an 
 * uploader phone at all. Think about it as a Parakeet V2. xDrip will still need to 
 * interpret this data & send it back to Nightscout but it will stay at home. It could 
 * be installed on an android tablet, a cellphone or even a simple Android emulator on 
 * a PC. This solution is perfect for parents who have small childs which are going to 
 * kindergarten or school and who don't want them to carry an expensive phone (xBridge2)
 * or activate a cellphone plan while they are 90% of the time within wifi coverage. 
 * (I actually have two T1D kids so having TWO cellphone data plans make it expensive 
 * for the low amount of data needed. I intend to allow 3G connexion as a backup.
 * 
 * The xBridge2 is available at the following address:
 * https://github.com/jstevensog/wixel-sdk/tree/master/apps/xBridge2
 * 
 * I would like to thanks John Stevens (jstevensog), StephenBlackWasAlreadyTaken and 
 * Adrien De Croy (Dexterity Project) for making this code possible
 * 
 * ( •_•)O*¯`·.¸.·´¯`°Q(•_• )
 * 
 * Want to support my work or help me with my 2 T1D kids?
 *   
 * My bitcoin address:
 * 1DKwRq8pbXQVADXHt17H1NLyqJrbGr8nDi
 * 
 * Paypal: 
 * paypal.me/YannickForget
 * 
 * created 24 Apr. 2016
 * by Yannick Forget
*/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include "WebServer.h"
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include "Configuration.h"
#include "DexcomHelper.h"

/*
 * FUNCTION PROTOTYPES (Needed since ESP8266WiFiMulti.h was included...)
 */
void StartWifiConnection();
void OpenDebugConnection();
void SendDebugText(String debugText);
void SendDebugText(char debugText);
void SendDebugText(char* debugText);
void SendDebugText(uint32_t debugText);
void SendDebugText(int debugText);
void ManageConnectionStarted(int receivedValue);
void ProcessWixelMessage(unsigned char* message);
void SendMessage(unsigned int messageId);
void SendMessage(unsigned int messageId, uint32_t messageContent);
void SendMessage(unsigned int messageId, char* messageContent);
void Sleep(int waitMillis);

/*
 * Wixel Configuration
 */
const String COMMUNICATION_STARTED_STRING = "AT+RESET";

/*
 * Wifi Configuration
 */

const char* DEBUG_HOST = "192.168.0.192";
const int DEBUG_PORT = 8001;

WiFiClient _debugClient;


/*
 * Protocol descriptions:
Data Packet - Bridge to App.  Sends the Dexcom transmitter data, and the bridge battery volts.
  0x11  - length of packet.
  0x00  - Packet type (00 means data packet)
  uint32  - Dexcom Raw value.
  uint32  - Dexcom Filtered value.
  uint8 - Dexcom battery value.
  uint16  - Bridge battery value.
  uint32  - Dexcom encoded TXID the bridge is filtering on.
  
Data Acknowledge Packet - App to Bridge.  Sends an ack of the Data Packet and tells the wixel to go to sleep.
  0x02  - length of packet.
  0xF0  - Packet type (F0 means acknowleged, go to sleep)
  
TXID packet - App to Bridge.  Sends the TXID the App wants the bridge to filter on.  In response to a Data packet or beacon packet being wrong.
  0x06  - Length of the packet.
  0x01  - Packet Type (01 means TXID packet).
  uint32  - Dexcom encoded TXID.
  
Beacon Packet - Bridge to App.  Sends the TXID it is filtering on to the app, so it can set it if it is wrong.
                Sent when the wixel wakes up, or as acknowledgement of a TXID packet.
  0x06  - Length of the packet.
  0xF1  - Packet type (F1 means Beacon or TXID acknowledge)
  uint32  - Dexcom encoded TXID.
 */
// All RX Message (From Wixel)
#define WIXEL_COMM_RX_DATA_PACKET 0x00 // The Wixel send this message when it receive Dexcom packet
#define WIXEL_COMM_RX_SEND_BEACON 0xF1 // The Wixel send this message when it wants to know if the Transmitter ID is ok
#define IS_DEBUG

// All TX Message (To Wixel)
#define WIXEL_COMM_TX_ACKNOWLEDGE_DATA_PACKET 0xF0 // This message send and acknowledge packet to allow Wixel to go in sleep mode
#define WIXEL_COMM_TX_SEND_TRANSMITTER_ID 0x01 // This message send the Transmitter ID to the Wixel
#define WIXEL_COMM_TX_SEND_DEBUG 0x64 // This message ask the Wixel to flip the Debug flag ON or OFF
#define WIXEL_COMM_TX_SLEEP_BLE 0x42 // This message ask the Wixel to flip the BLE Sleeping flag ON or OFF
#define WIXEL_COMM_TX_DO_LED 0x4C // This message ask the Wixel to flip the Led Sleeping flag ON or OFF

#define DEXBRIDGE_PROTO_LEVEL 0x01


#define HTTP_PORT 80
typedef struct Dexcom_Packet_Struct
{
  uint8_t len;
  uint32_t  dest_addr;
  uint32_t  src_addr;
  uint8_t port;
  uint8_t device_info;
  uint8_t txId;
  uint16_t  raw;
  uint16_t  filtered;
  uint8_t battery;
  uint8_t unknown;
  uint8_t checksum;
  int8_t  RSSI;
  uint8_t LQI;
} Dexcom_packet;

// structure of a raw record we receive from the Wixel.
typedef struct Wixel_RawRecord_Struct
{
  uint32_t  raw;  //"raw" BGL value.
  uint32_t  filtered; //"filtered" BGL value 
  uint8_t dex_battery;  //battery value
  uint8_t my_battery; //xBridge battery value
  uint32_t  dex_src_id;   //raw TXID of the Dexcom Transmitter
  //int8  RSSI; //RSSI level of the transmitter, used to determine if it is in range.
  //uint8 txid; //ID of this transmission.  Essentially a sequence from 0-63
  uint8_t function; // Byte representing the xBridge code funcitonality.  01 = this level.
} RawRecord;

int _messageLength = 0;
int _messagePosition = 0;
unsigned char* _message;

WebServer _webServer;
Configuration _configuration;
DexcomHelper _dexcomHelper;
/*
 * Function: setup
 * ---------------
 * Setup method called once when starting the Arduino
 */
void setup() {
  // Open serial communications
  Serial.begin(9600);
  EEPROM.begin(4096); // Use maximum allowed size
  /*while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }*/
  //String.toCharArray(_configuration.getAppEngineAddress(), 
  _webServer.start();
  StartWifiConnection();
  OpenDebugConnection();
  #ifdef IS_DEBUG
  SendDebugText("wifi-xBridge Started!\r\nDebugging mode ON\r\n");
  #endif
}

/*
 * StartWifiConnection
 * -------------------
 * This method starts the Wifi Connection
 * Todo: This should use a dynamic list of wifi
 */
void StartWifiConnection(){
  
  ESP8266WiFiMulti wifiMulti;
  //Serial.write("StartWifiConnection");
  // Open WiFi connection
  if (_configuration.getWifiCount() > 0)
  {
    //Serial.print(_configuration.getWifiCount());
    //Serial.print("WiFi found\r\n");
    for(int i = 0 ; i < _configuration.getWifiCount(); i++)
    {
      WifiData* data = _configuration.getWifiData(i);
      int ssidLength = data->ssid.length() + 1;
      int passwordLength = data->password.length() + 1;
      char ssidChars[ssidLength + 1];
      char passwordChars[passwordLength + 1];
      /*Serial.print(_configuration.getWifiCount());
      Serial.print("Adding Wifi:\r\n");
      Serial.print(data->ssid);
      Serial.print("|");
      Serial.print(data->password);
      Serial.print("\r\n");*/
      data->ssid.toCharArray(ssidChars, ssidLength);
      data->password.toCharArray(passwordChars, passwordLength);
      wifiMulti.addAP(ssidChars, passwordChars);
    }
    int connectionTry = 0;
    //Serial.print("Trying to connect");
    
    while (WiFi.status() != WL_CONNECTED && connectionTry < 120) {
      connectionTry++;
      if(wifiMulti.run() == WL_CONNECTED) {
        //Serial.print("Connected\r\n");
      }
      else
      {
        //Serial.print("Failed\r\n");
        if (connectionTry % 5 == 0)
        {
          //Serial.print("Not connected :(\r\n");
        }
        //Serial.print("Server Loop\r\n");
        _webServer.loop();
        Sleep(500);
      }
    }
  }
}

void Sleep(int waitMillis)
{
  int startMillis = millis();
  while (millis() < waitMillis + startMillis)
  {
    // Still use webserver loop
    _webServer.loop();
  }
  
}

/*
 * StopWifiConnection
 * ------------------
 * This method should stop the wifi connection
 */
void StopWifiConnection() {
  WiFi.disconnect(true);
}

/*
 * Function: OpenDebugConnection
 * -----------------------------
 * This method will open the connection with a dummy debug server
 */
void OpenDebugConnection(){
  // Open DEBUG connection
  _debugClient.connect(DEBUG_HOST, DEBUG_PORT);
  SendDebugText("Connected!\r\n");
}

/*
 * Function: CloseDebugConnection
 * ------------------------------
 * This method will close the connection with the debug server
 */
void CloseDebugConnection() {
  #ifdef IS_DEBUG
  SendDebugText("Closing connection. Bye!\r\n");
  #endif
  // Close DEBUG connection
  _debugClient.stop();
}


/*
 * Function: loop
 * --------------
 * Classic Arduino Loop method
 */
void loop() {
  _webServer.loop();
  // Check is there is data on RX port from Wixel
  while (Serial.available() > 0) {
    int receivedValue = Serial.read();
    // Display data for debugging
    #ifdef IS_DEBUG
    SendDebugText("Received: ");
    char parsedText[5];
    _dexcomHelper.IntToCharArray(receivedValue, parsedText);
    SendDebugText(parsedText);
    SendDebugText(" (");
    SendDebugText(char(receivedValue));
    SendDebugText(")");
    SendDebugText("\r\n");
    #endif
    ManageConnectionStarted(receivedValue);
  }

  // If there is data comming from Serial port, send it back to the Wixel (For debugging purpose)
  /*while (Serial.available() > 0) {
    unsigned char inByte = Serial.read();
    Serial.write(inByte);
    Serial.print(inByte);
  }*/
}


/*
 * Function SendDebugText
 * ----------------------
 * This method is used to send DEBUG text by Wifi or Serial
 * debugText: The text to be sent
 */
void SendDebugText(String debugText){
  #ifdef IS_DEBUG
  if (WiFi.status() == WL_CONNECTED) {
    _debugClient.print(debugText);
  }
  //Serial.print(debugText);
  CloseDebugConnection();
  #endif
}

/*
 * Function SendDebugText
 * ----------------------
 * This method is used to send DEBUG text by Wifi or Serial
 * debugText: The text to be sent
 */
void SendDebugText(char debugText){
  #ifdef IS_DEBUG
  if (WiFi.status() == WL_CONNECTED) {
    _debugClient.print(debugText);
  }
  //Serial.write(debugText);
  #endif
}

/*
 * Function SendDebugText
 * ----------------------
 * This method is used to send DEBUG text by Wifi or Serial
 * debugText: The text to be sent
 */
void SendDebugText(char* debugText){
  #ifdef IS_DEBUG
  if (WiFi.status() == WL_CONNECTED) {
    _debugClient.print(debugText);
  }
  //Serial.write(debugText);
  #endif
}

void SendDebugText(uint32_t debugText){
  #ifdef IS_DEBUG
  if (WiFi.status() == WL_CONNECTED) {
    _debugClient.print(debugText);
  }
  //Serial.write(debugText);
  #endif
}

void SendDebugText(int debugText){
  #ifdef IS_DEBUG
  if (WiFi.status() == WL_CONNECTED) {
    _debugClient.print(debugText);
  }
  //Serial.write(debugText);
  #endif
}

long timeElapsedLastReception;
// delay in milliseconds for maximum time between reception of message
unsigned int maxIntervalBetweenReception = 2000;

/*
 * Function: ManageConnectionStarted
 * ---------------------------------
 * We just received a byte from the Wixel and the real communication is already started
 * 
 * receivedValue: The value we just received on Serial Port
*/
void ManageConnectionStarted(int receivedValue) {
  long currentTimeElapsed = millis();
  if (currentTimeElapsed > timeElapsedLastReception + maxIntervalBetweenReception) 
  {
    // Reset message reception
    _messageLength = 0;
    _messagePosition = 0;
  }
  if (_messageLength <= 0) {
    if (receivedValue == 0) // 0 length message...impossible skip
    {
      return;
    }
    // First byte is to determine the message length
    _messageLength = receivedValue;
    _message = new unsigned char[_messageLength];
    _messagePosition = 0;
  }
  _message[_messagePosition] = receivedValue;
  _messagePosition++;
  if (_messagePosition == _messageLength)
  {
    #ifdef IS_DEBUG
    // We have a complete messsage to process
    SendDebugText("Looks like we have a full message to process! (");
    char textNbChar [5];
    _dexcomHelper.IntToCharArray((int)_message[0], textNbChar);
    SendDebugText(textNbChar);
    SendDebugText(" characters) \r\n");
    #endif
    // Process message
    ProcessWixelMessage(_message);
    // Reset message vars
    _messageLength = 0;
  }
  timeElapsedLastReception = currentTimeElapsed;
}

/*
 * Function: CharArrayToInt32
 * --------------------------
 * This function convert a char array to a uint32_t type
 */
uint32_t CharArrayToInt32(char* array)
{
  return ((uint32_t)array[0] << 0) || 
         ((uint32_t)array[1] << 8) ||
         ((uint32_t)array[2] << 16) ||
         ((uint32_t)array[3] << 24);
/*  uint32_t returnValue;
  memcpy(&returnValue, array, 4);
  return returnValue;*/
}


long lastTransmission;
/*
 * Function: SendAppEngineData
 * ---------------------------
 * This function will send the Wixel Data to the Google App Engine
 * 
 * dexcomData: This is the Dexcom data to send to the AppEngine
 */
void SendAppEngineData(struct Wixel_RawRecord_Struct dexcomData)
{
  uint32_t transmitterId = dexcomData.dex_src_id;
  #ifdef IS_DEBUG
  SendDebugText("Preparing to send data to App Engine:\r\n");
  #endif

  String appEngineAddress = _configuration.getAppEngineAddress();
  char appEngineCharArray[appEngineAddress.length() + 1];
  appEngineAddress.toCharArray(appEngineCharArray, appEngineAddress.length() + 1);
  char* appEngineHost = appEngineCharArray;
  /*Serial.print("AppEngineHost: ");
  Serial.print(appEngineHost);
  Serial.print("\r\n");*/

  WiFiClient client;
  if (!client.connect(appEngineHost, HTTP_PORT)) {
    #ifdef IS_DEBUG
    SendDebugText("Can't connect to App Engine :(\r\n");
    #endif
  }
  else
  {
    #ifdef IS_DEBUG
    SendDebugText("Connected to App Engine :)\r\n\r\n");
    #endif
  }
  #ifdef IS_DEBUG
  SendDebugText("Sending data to App Engine:\r\n");
  #endif
  String url = "/receiver.cgi?zi=";
  url += transmitterId;
  url += "&pc="; 
  url += "0"; // Passcode
  url += "&lv=";
  url += dexcomData.raw; // Raw Data
  url += "&lf=";
  url += dexcomData.filtered; // Filtered Data
  url += "&db=";
  url += dexcomData.dex_battery; // Battery (Dexcom)
  url += "&ts=";
  url += millis() - lastTransmission; // Capture Date Time str(int(time.time()) - (int(data.ts) / 1000)) + "000"
  url += "&bp=";
  url += dexcomData.my_battery; // Uploader Battery Life
  url += "&bm=";
  url += 3755; // ?
  url += "&ct=";
  url += 22; // ?
  url += "&gl="; 
  url += 0; // Geolocation ?

  #ifdef IS_DEBUG
  SendDebugText("Sending data: ");
  SendDebugText(url);
  SendDebugText("\r\n");
  #endif
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + appEngineHost + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      #ifdef IS_DEBUG
      SendDebugText(">>> Client Timeout !\r\n");
      #endif
      client.stop();
      return;
    }
  }

  #ifdef IS_DEBUG
  // Read all the lines of the reply from server and print them to Debug if available
  while(client.available()){
    String line = client.readStringUntil('\0');
    SendDebugText(line);
  }
  #endif
  SendDebugText("\r\nclosing connection\r\n");
  client.stop();
  lastTransmission = millis();
}

/*
 * Function: ProcessWixelMessage
 * -----------------------------
 * We now have a complete message and we need to process it
 * 
 * message: The message to process 
 */
void ProcessWixelMessage(unsigned char* message)
{
  #ifdef IS_DEBUG
  OpenDebugConnection();
  #else
  StartWifiConnection();
  #endif
  unsigned int messageLength = message[0];
  unsigned int messageType = (int)message[1];
  #ifdef IS_DEBUG
  SendDebugText("Message type to process:");
  SendDebugText(message[1]);
  SendDebugText(":");
  SendDebugText((unsigned int)message[1]);
  #endif
  switch(messageType)
  {
    case WIXEL_COMM_RX_DATA_PACKET:
      SendDebugText("We received a Dexcom Data Packet w00t!\r\n");
      SendMessage(WIXEL_COMM_TX_ACKNOWLEDGE_DATA_PACKET);
      struct Wixel_RawRecord_Struct dexcomData;
      //memcpy(&dexcomData, &message[2], sizeof(dexcomData)); //messageLength - 2);
      
      dexcomData.raw = message[5];
      dexcomData.raw = (dexcomData.raw << 8) + message[4];
      dexcomData.raw = (dexcomData.raw << 8) + message[3];
      dexcomData.raw = (dexcomData.raw << 8) + message[2];
      
      dexcomData.filtered = message[9];
      dexcomData.filtered = (dexcomData.filtered << 8) + message[8];
      dexcomData.filtered = (dexcomData.filtered << 8) + message[7];
      dexcomData.filtered = (dexcomData.filtered << 8) + message[6];
      
      dexcomData.dex_battery = message[10];
      
      dexcomData.my_battery = message[11];
      
      dexcomData.dex_src_id = message[15];
      dexcomData.dex_src_id = (dexcomData.dex_src_id << 8) + message[14];
      dexcomData.dex_src_id = (dexcomData.dex_src_id << 8) + message[13];
      dexcomData.dex_src_id = (dexcomData.dex_src_id << 8) + message[12];

      dexcomData.function = message[16];
      
      SendDebugText("\r\nraw: ");
      SendDebugText(dexcomData.raw);
      SendDebugText("\r\nfiltered: ");
      SendDebugText(dexcomData.filtered);
      SendDebugText("\r\ndex_battery: ");
      SendDebugText(dexcomData.dex_battery);
      SendDebugText("\r\nmy_battery: ");
      SendDebugText(dexcomData.my_battery);
      SendDebugText("\r\ndex_src_id: ");
      SendDebugText(dexcomData.dex_src_id);
      SendDebugText("\r\nfunction: ");
      SendDebugText(dexcomData.function);
      SendAppEngineData(dexcomData);
    case WIXEL_COMM_RX_SEND_BEACON:
      if(messageLength == 7){
        // Spit the Wixel's Transmitter ID out of the message
        uint32_t transmitterIdSrc;
        memcpy(&transmitterIdSrc, &message[2], 4);

        #ifdef IS_DEBUG
        SendDebugText("Transmitter ID Src:");
        SendDebugText(transmitterIdSrc);
        SendDebugText("\r\n");
        #endif
        uint32_t configuredTransmitterId = _configuration.getTransmitterId();
        char* configuredTransmitterIdAscii = _dexcomHelper.DexcomSrcToAscii(configuredTransmitterId);
        char* transmitterIdAscii = _dexcomHelper.DexcomSrcToAscii(transmitterIdSrc);
        #ifdef IS_DEBUG
        SendDebugText("Wixel thinks the transmitter ID is: ");
        SendDebugText(transmitterIdAscii);
        SendDebugText("\r\n");
        #endif
        // Check if it's the proper transmitter ID
        
        if (strcmp(transmitterIdAscii, configuredTransmitterIdAscii) == 0)
        {
          SendDebugText("Good, the Wixel has proper transmitter ID\r\n");
        }
        else
        {
          SendDebugText("Lol, send the proper Transmitter ID to the Wixel right now!\r\n");
          SendMessage(WIXEL_COMM_TX_SEND_TRANSMITTER_ID, configuredTransmitterId);
        }
        free(transmitterIdAscii);
        free(configuredTransmitterIdAscii);
      }
      break;
    default:
      SendDebugText("Unkown message :/");
      SendDebugText(messageType);
      SendDebugText("\r\n");
  }
  #ifdef IS_DEBUG
  CloseDebugConnection();
  #else
  StopWifiConnection();
  #endif
  
}

/*
 * Function: SendMessage
 * ---------------------
 * This method is used to send a message
 * messageId: The message ID to send
 */
void SendMessage(unsigned int messageId)
{
  unsigned int messageLength = 2; // Message length byte + message id byte
  char textNbChar [5];
  _dexcomHelper.IntToCharArray(messageLength, textNbChar);
  SendDebugText("Message Length: ");
  SendDebugText(textNbChar);
  SendDebugText("\r\n");
  Serial.write(messageLength);
  Serial.write(messageId);
  SendDebugText("Message ID: ");
  SendDebugText(messageId);
  SendDebugText("\r\n");
}

/*
 * Function: SendMessage
 * ---------------------
 * This method is used to send a message
 * messageId: The message ID to send
 * messageContent: The message content in uint32_t format (Used for TransmitterId in Src format)
 */
void SendMessage(unsigned int messageId, uint32_t messageContent)
{
  unsigned int messageLength = 6; // Message content (uint32_t = 4 bytes) + message length byte + message id byte
  char textNbChar [5];
  _dexcomHelper.IntToCharArray(messageLength, textNbChar);
  SendDebugText("Message Length: ");
  SendDebugText(textNbChar);
  SendDebugText("\r\n");
  Serial.write(messageLength);
  Serial.write(messageId);
  SendDebugText("Message ID: ");
  SendDebugText(messageId);
  SendDebugText("\r\n");
  Serial.write( lowByte(messageContent) );
  Serial.write( lowByte(messageContent >> 8) );
  Serial.write( lowByte(messageContent >> 16) );
  Serial.write( lowByte(messageContent >> 24) );
  SendDebugText("Message Content: ");
  SendDebugText(messageContent);
  SendDebugText("\r\n");
  SendDebugText( lowByte(messageContent) );
  SendDebugText( lowByte(messageContent >> 8) );
  SendDebugText( lowByte(messageContent >> 16) );
  SendDebugText( lowByte(messageContent >> 24) );
  SendDebugText("\r\n");
}

// 
/*
 * Function: SendMessage
 * ---------------------
 * This method is used to send a message
 * messageId: The message ID to send
 * messageContent: The message content when available
 */
void SendMessage(unsigned int messageId, char* messageContent)
{
  unsigned int messageLength = strlen(messageContent) + 2; // Message content + message length byte + message id byte
  char textNbChar [5];
  _dexcomHelper.IntToCharArray(messageLength, textNbChar);
  SendDebugText("Message Length: ");
  SendDebugText(textNbChar);
  SendDebugText("\r\n");
  Serial.write(messageLength);
  
  Serial.write(messageId);
  Serial.write(messageContent);
}


