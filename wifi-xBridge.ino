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
const String COMMUNICATION_STARTED_STRING = "AT+RESET";
const char* DEXCOM_TRANSMITTER_ID = "6BCFK\0";

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
  0xF0  - Packet type (F0 means acknowleged, go to sleep.
  
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
#define WIXEL_COMM_RX_SEND_BEACON 0xF1 // The Wixel send this message when starting to know if the Transmitter ID is ok
#define IS_DEBUG

// All TX Message (To Wixel)
#define WIXEL_COMM_TX_ACKNOWLEDGE_DATA_PACKET 0xF0 // This message send and acknowledge packet to allow Wixel to go in sleep mode
#define WIXEL_COMM_TX_SEND_TRANSMITTER_ID 0x01 // This message send the Transmitter ID to the Wixel
#define WIXEL_COMM_TX_SEND_DEBUG 0x64 // This message ask the Wixel to send debug output ON or OFF

#define DEXBRIDGE_PROTO_LEVEL 0x01

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

// software serial #1: RX = digital pin 2, TX = digital pin 3
SoftwareSerial uartPort(2, 3);

// We need to wait for the AT+RESET command before any real communication with
// the xBridge. These parameters will keep track of this.
bool _communicationStarted = false;
int _communicationStartedCharacter = 0;
int _messageLength = 0;
int _messagePosition = 0;
unsigned char* _message;

/*
 * Function: setup
 * ---------------
 * Setup method called once when starting the Arduino
 */
void setup() {
  // Open serial communications for debugging
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("wifi-xBridge Started!\r\n");
  Serial.print("Debugging mode ");
  #ifdef IS_DEBUG
  Serial.print("ON");
  #else
  Serial.print("OFF");
  #endif
  Serial.print("\r\n");
  // Start software serial port for wixel communication
  uartPort.begin(9600);
  uartPort.listen();
  _communicationStarted = true;
}

/*
 * Function: loop
 * --------------
 * Classic Arduino Loop method
 */
void loop() {
  // Check is there is data on RX port from Wixel
  while (uartPort.available() > 0) {
    int receivedValue = uartPort.read();
    // Display data for debugging
    #ifdef IS_DEBUG
    Serial.write("Received: ");
    char parsedText[5];
    IntToCharArray(receivedValue, parsedText);
    Serial.write(parsedText);
    Serial.write(" (");
    Serial.write(char(receivedValue));
    Serial.write(")");
    Serial.println();
    #endif
    // We should wait for the AT+RESET message before parsing anything else. 
    // I don't see the point of using any other character before that for now.
    if (!_communicationStarted) {
      ManageConnectionNotStarted(receivedValue);
    }
    else {
      ManageConnectionStarted(receivedValue);
    }
  }

  // If there is data comming from Serial port, send it back to the Wixel (For debugging purpose)
  while (Serial.available() > 0) {
    unsigned char inByte = Serial.read();
    uartPort.write(inByte);
    Serial.print(inByte);
  }
}

/* All possible values of Dexcom Transmitter ID Characters*/
const char SRC_NAME_TABLE[32] = { '0', '1', '2', '3', '4', '5', '6', '7',
              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
              'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P',
              'Q', 'R', 'S', 'T', 'U', 'W', 'X', 'Y' };
/*
 * Function: DexcomSrcToAscii
 * -------------------------
 * Transform the uint32_t value of the Dexcom transmitter back to Ascii format
 * src: The special src value representing the Transmitter ID
 */
char* DexcomSrcToAscii(uint32_t src)
{
  char* transmitterId = (char*) malloc(sizeof(char) * 6);
  transmitterId[0] = SRC_NAME_TABLE[(src >> 20) & 0x1F];
  transmitterId[1] = SRC_NAME_TABLE[(src >> 15) & 0x1F];
  transmitterId[2] = SRC_NAME_TABLE[(src >> 10) & 0x1F];
  transmitterId[3] = SRC_NAME_TABLE[(src >> 5) & 0x1F];
  transmitterId[4] = SRC_NAME_TABLE[(src >> 0) & 0x1F];
  transmitterId[5] = '\0';
  return transmitterId;
}

/*
 * Function: DexcomAsciiToSrc
 * -------------------------
 * Transform the Dexcom Ascii format to Src value (unsigned int32)
 * transmitterId: The transmitter Id to transform
 * returns: The src value corresponding to this DexCom Transmitter ID
 */
uint32_t DexcomAsciiToSrc(char* transmitterId)
{
  uint32_t returnValue = 0;
  returnValue ^= TransmitterIdCharacterNumber(transmitterId[0]) << 20;
  returnValue ^= TransmitterIdCharacterNumber((uint32_t)transmitterId[1]) << 15;
  returnValue ^= TransmitterIdCharacterNumber((uint32_t)transmitterId[2]) << 10;
  returnValue ^= TransmitterIdCharacterNumber((uint32_t)transmitterId[3]) << 5;
  returnValue ^= TransmitterIdCharacterNumber((uint32_t)transmitterId[4]) << 0;
  return returnValue;
}

/*
 * Function: TransmitterIdCharacterNumber
 * --------------------------------------
 * Return the character number of the specifier char
 * character: Transmitter ID character
 * returns: Number of that character in the SRC_NAME_TABLE array
 */
uint32_t TransmitterIdCharacterNumber(char character)
{
  for(int i = 0; i < 32; i++)
  {
    if (SRC_NAME_TABLE[i] == character)
    {
      return i;
    }
  }
}

/*
 * Function: IntToCharArray
 * ------------------------
 * Transform value from int to a char array [5]
 * 
 * value: The int value to transform
 * result: The char array to write to
 */
void IntToCharArray(unsigned int value, char* result) {
  String textValue = String(value);
  textValue.toCharArray(result, 5);
}

/*
 * Function: ManageConnectionNotStarted
 * ------------------------------------
 * We just received a byte from the Wixel but the real communication is not started yet (Wait for AT+RESET)
 * 
 * receivedValue: The value we just received on Serial Port
 */
void ManageConnectionNotStarted(int receivedValue) {
  if (COMMUNICATION_STARTED_STRING[_communicationStartedCharacter] == char(receivedValue)) {
    _communicationStartedCharacter++;
  }
  else{
    _communicationStartedCharacter = 0;
  }
  if(COMMUNICATION_STARTED_STRING.length() == _communicationStartedCharacter)
  {
    _communicationStarted = true;
    #ifdef IS_DEBUG
    Serial.write("AT+RESET was received. Wixel communication officially started\r\n");
    #endif
  }
}

/*
 * Function: ManageConnectionStarted
 * ---------------------------------
 * We just received a byte from the Wixel and the real communication is already started
 * 
 * receivedValue: The value we just received on Serial Port
*/
void ManageConnectionStarted(int receivedValue) {
  if (_messageLength <= 0) {
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
    Serial.print("Looks like we have a full message to process! (");
    char textNbChar [5];
    IntToCharArray((int)_message[0], textNbChar);
    Serial.print(textNbChar);
    Serial.print(" characters) \r\n");
    #endif
    // Process message
    ProcessWixelMessage(_message);
    // Reset message vars
    _messageLength = 0;
  }
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

/*
 * Function: ProcessWixelMessage
 * -----------------------------
 * We now have a complete message and we need to process it
 * 
 * message: The message to process 
 */
void ProcessWixelMessage(unsigned char* message)
{
  unsigned int messageLength = message[0];
  unsigned int messageType = (int)message[1];
  #ifdef IS_DEBUG
  Serial.print("Message type to process:");
  Serial.print(message[1]);
  Serial.print(":");
  Serial.print((unsigned int)message[1]);
  #endif
  switch(messageType)
  {
    case WIXEL_COMM_RX_DATA_PACKET:
      Serial.print("We received a Dexcom Data Packet w00t!\r\n");
      struct Dexcom_Packet_Struct dexcomData;
      memcpy(&dexcomData, &message[2], sizeof(dexcomData)); //messageLength - 2);
      /*char intCharSource[5];
      dexcomData.len = message[2];
      intCharSource[0] = message[3];
      intCharSource[1] = message[4];
      intCharSource[2] = message[5];
      intCharSource[3] = message[6];*/
      //strncpy(intCharSource, (char*)message[3], 4);
      /*Serial.print("int char source:");
      Serial.print(intCharSource);
      Serial.print("\r\n");*/
      //dexcomData.dest_addr = CharArrayToInt32(intCharSource);
      //strncpy(intCharSource, (char*)message[7], 4);
      //dexcomData.src_addr = CharArrayToInt32(intCharSource);
      Serial.print("Dest_Addr: ");
      Serial.print(dexcomData.dest_addr);
      Serial.print("\r\nSrc_Addr: ");
      Serial.print(dexcomData.src_addr);
      Serial.print("\r\nPort: ");
      Serial.print(dexcomData.port);
      Serial.print("\r\nDevice Info: ");
      Serial.print(dexcomData.device_info);
      Serial.print("\r\ntxId: ");
      Serial.print(dexcomData.txId);
      Serial.print("\r\nraw: ");
      Serial.print(dexcomData.raw);
      Serial.print("\r\nfiltered: ");
      Serial.print(dexcomData.filtered);
      Serial.print("\r\nbattery: ");
      Serial.print(dexcomData.battery);
      Serial.print("\r\nunknown: ");
      Serial.print(dexcomData.unknown);
      Serial.print("\r\nchecksum: ");
      Serial.print(dexcomData.checksum);
      Serial.print("\r\nRSSI: ");
      Serial.print(dexcomData.RSSI);
      Serial.print("\r\nLQI: ");
      Serial.print(dexcomData.LQI);
      Serial.print("\r\n");
    case WIXEL_COMM_RX_SEND_BEACON:
      if(messageLength == 7){
        // Spit the Wixel's Transmitter ID out of the message
        uint32_t transmitterIdSrc;
        memcpy(&transmitterIdSrc, &message[2], 4);
        /*uint32_t transmitterIdSrc = ((uint32_t)message[2] << 0) ||
                           ((uint32_t)message[3] << 8) ||
                           ((uint32_t)message[4] << 16) ||
                           ((uint32_t)message[5] << 24);*/
        
        //memcpy(&transmitterIdSrc, (char*)message[2], 4);
        #ifdef IS_DEBUG
        Serial.print("Transmitter ID Src:");
        Serial.print(transmitterIdSrc);
        Serial.print("\r\n");
        #endif
        char* transmitterIdAscii = DexcomSrcToAscii(transmitterIdSrc);
        #ifdef IS_DEBUG
        Serial.print("Wixel thinks the transmitter ID is: ");
        Serial.print(transmitterIdAscii);        
        Serial.print("\r\n");
        #endif
        // Check if it's the proper transmitter ID
        if (strcmp(transmitterIdAscii, DEXCOM_TRANSMITTER_ID) == 0)
        {
          Serial.print("Good, the Wixel has proper transmitter ID\r\n");
        }
        else
        {
          Serial.print("Lol, send the proper Transmitter ID to the Wixel right now!\r\n");
          uint32_t dexcomTrxIDSrc = DexcomAsciiToSrc((char*)DEXCOM_TRANSMITTER_ID);
          SendMessage(WIXEL_COMM_TX_SEND_TRANSMITTER_ID, dexcomTrxIDSrc);
        }
        free(transmitterIdAscii);
      }
      break;
    default:
      Serial.print("Unkown message :/");
      Serial.print(messageType);
      Serial.print("\r\n");
  }
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
  IntToCharArray(messageLength, textNbChar);
  Serial.print("Message Length: ");
  Serial.print(textNbChar);
  Serial.print("\r\n");
  uartPort.write(messageLength);
  uartPort.write(messageId);
  Serial.print("Message ID: ");
  Serial.print(messageId);
  Serial.print("\r\n");
  uartPort.write( lowByte(messageContent) );
  uartPort.write( lowByte(messageContent >> 8) );
  uartPort.write( lowByte(messageContent >> 16) );
  uartPort.write( lowByte(messageContent >> 24) );
  Serial.print("Message Content: ");
  Serial.print(messageContent);
  Serial.print("\r\n");
  Serial.write( lowByte(messageContent) );
  Serial.write( lowByte(messageContent >> 8) );
  Serial.write( lowByte(messageContent >> 16) );
  Serial.write( lowByte(messageContent >> 24) );
  Serial.print("\r\n");
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
  IntToCharArray(messageLength, textNbChar);
  Serial.print("Message Length: ");
  Serial.print(textNbChar);
  Serial.print("\r\n");
  uartPort.write(messageLength);
  
  uartPort.write(messageId);
  uartPort.write(messageContent);
}


