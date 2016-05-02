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
 * I would like to thanks John Stevens (jstevensog), StephenBlackWasAlreadyTaken and Adrien De Croy (Dexterity Project) for making this code possible
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

#define WIXEL_COMM_RX_SEND_BEACON 0xF1 // The Wixel send this message when starting to know if the Transmitter ID is ok

#define WIXEL_COMM_TX_ACKNOWLEDGE_DATA_PACKET 0xF0 // This message send and acknowledge packet to allow Wixel to go in sleep mode
#define WIXEL_COMM_TX_SEND_TRANSMITTER_ID 0x01 // This message send the Transmitter ID to the Wixel
#define WIXEL_COMM_TX_SEND_DEBUG 0x64 // This message ask the Wixel to send debug output ON or OFF

// software serial #1: RX = digital pin 2, TX = digital pin 3
SoftwareSerial uartPort(2, 3);

// We need to wait for the AT+RESET command before any real communication with
// the xBridge. These parameters will keep track of this.
bool _communicationStarted = false;
int _communicationStartedCharacter = 0;
int _messageLength = 0;
int _messagePosition = 0;
unsigned char* _message;
const String COMMUNICATION_STARTED_STRING = "AT+RESET";
const char DEXCOM_TRANSMITTER_ID[] = "6BCFK\0";

// Setup method called once when starting the Arduino
void setup() {
  // Open serial communications for debugging
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // Start software serial port for wixel communication
  uartPort.begin(9600);
  uartPort.listen();
}

// Loop method
void loop() {
  // Check is there is data on RX port from Wixel
  while (uartPort.available() > 0) {
    int receivedValue = uartPort.read();
    // Display data for debugging
    Serial.write("Received: ");
    char parsedText[5];
    IntToCharArray(receivedValue, parsedText);
    Serial.write(parsedText);
    Serial.write(" (");
    Serial.write(char(receivedValue));
    Serial.write(")");
    Serial.println();
    // We should wait for the AT+RESET message before parsing anything else. I don't see the point of using any character before that for now.
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

// Transform value from int to a char array [5]
void IntToCharArray(int value, char* result) {
  String textValue = String(value);
  textValue.toCharArray(result, 5);
}

// We just received a byte from the Wixel but the real communication is not started yet (Wait for AT+RESET)
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
    Serial.write("AT+RESET was received. Wixel communication officially started\r\n");
  }
}

// We just received a byte from the Wixel and the real communication is already started
/*
 * Function: ManageConnectionStarted
 * ---------------------------------
 * We just received a byte from the Wixel and the real communication is already started
 * 
 * receivedValue: The value we just received
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
    // We have a complete messsage to process w00t!
    Serial.print("Looks like we have a full message to process! (");
    char textNbChar [5];
    IntToCharArray((int)_message[0], textNbChar);
    Serial.print(textNbChar);
    Serial.print(" characters) \r\n");
    // Process message
    ProcessWixelMessage(_message);
    // Reset message vars
    _messageLength = 0;
  }
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
  Serial.print("Message type to process:");
  Serial.print(message[1]);
  Serial.print(":");
  Serial.print((unsigned int)message[1]);
  switch(messageType)
  {
    case WIXEL_COMM_RX_SEND_BEACON:
      if(messageLength > 6){
        // Spit the Wixel's Transmitter ID out of the message
        char transmitterId[5];
        memcpy(transmitterId, &message[2], 5);
        Serial.print("Wixel thinks the transmitter ID is: ");
        if (transmitterId[0] == 0)
        {
          Serial.print("Nothing!");
        }
        else
        {
          Serial.print(transmitterId);
        }
        
        Serial.print("\r\n");
        // Check if it's the proper transmitter ID
        if (strcmp(transmitterId, DEXCOM_TRANSMITTER_ID) == 0)
        {
          Serial.print("Good, the Wixel has proper transmitter ID");
        }
        else
        {
          Serial.print("Lol, send the proper Transmitter ID to the Wixel right now");
          SendMessage(WIXEL_COMM_TX_SEND_TRANSMITTER_ID, DEXCOM_TRANSMITTER_ID);
        }
      }
      break;
    default:
      Serial.print("Unkown message :/");
      Serial.print(messageType);
      Serial.print("\r\n");
  }
}

// This method is used to send a message
void SendMessage()
{
  
}

