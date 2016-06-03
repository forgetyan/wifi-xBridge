/*
 * Configuration.c - Library for managing configuration reading and saving in the EEPROM memory
 */


/*
 * The data structure is as follow
 * 
 * 1st character is ¶ to ensure that data is valid
 * 4 next chars are for TransmitterId in uint32_t format
 * 
 * Next characters until '¬' character is the App engine address
 * 
 * Next configs are wifi SSID and password all separated by '¬'(0xAC) character and will finish with NUL (0x00) character 
 * Wifi SSID ¬ Wifi Password ¬
 * Wifi 2 SSID ¬ Wifi 2 Password ¬
 * Wifi 3 SSID ¬ Wifi 3 Password (NUL)
 * ex:
 * ¶2g1bmyaddress.appspot.com¬wifi1¬password1¬wifi2¬password2·
 */
 
#include "Configuration.h"

const static char CONFIGURATION_SEPARATOR = '¬';
DexcomHelper Configuration::_dexcomHelper;

/*
 * Constructor
 */
Configuration::Configuration()
{
  _loaded = false;
}
/*
 * This method will save the transmitter Id to the EEPROM
 */
void Configuration::setTransmitterId(uint32_t transmitterId) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  Serial.print("setTransmitterId: ");
  Serial.print(transmitterId);
  Serial.print("\r\n");
  bridgeConfig->transmitterId = transmitterId;
  //EEPROM_writeAnything(1, transmitterId);
}

/*
 * This method will get the transmitter Id from the EEPROM
 */
uint32_t Configuration::getTransmitterId() {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  Serial.print("getTransmitterId: ");
  Serial.print(bridgeConfig->transmitterId);
  Serial.print("\r\n");
  return bridgeConfig->transmitterId;
  /*
  uint32_t transmitterId;
  //byte value = EEPROM.read(1);
  //transmitterId = value;
  EEPROM_readAnything(1, transmitterId);
  return transmitterId;*/
}

/*
 * This method will get the Google App Engine Address
 */
String Configuration::getAppEngineAddress() {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  
  return bridgeConfig->appEngineAddress;
}

/*
 * This method will set the Google App Engine Address
 */
void Configuration::setAppEngineAddress(String value) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  
  bridgeConfig->appEngineAddress = value;
}

/*
 * Configuration::getBridgeConfig
 * ------------------------------
 * This method will get the bridge configuration and load it if neccesary
 */
BridgeConfig* Configuration::getBridgeConfig() {
  if(!_loaded){
    _bridgeConfig = LoadConfig();
  }
  return _bridgeConfig;
}

/*
 * Configuration::LoadConfig
 * -------------------------
 * This method will load the configuration object from EEPROM
 * returns: The bridge configuration struct
 */
BridgeConfig* Configuration::LoadConfig() {
  Serial.print("Load configuration");
  BridgeConfig* config = (BridgeConfig*)malloc(sizeof(BridgeConfig));// BridgeConfig();
  String eepromData;
  bool continueReading = true;
  bool separatorFound = false;
  bool readingSSID = true;
  bool appEngineRead = false;
  char firstChar = EEPROM.read(0);
  String nextSSID = "";
  String nextPassword = "";
  //if (firstChar == '¶') {
    Serial.print("Configuration Valid");
    // Configuration is valid
    int i = 1;
    while(continueReading) {
      byte newChar = EEPROM.read(i);
      Serial.print("Char: ");
      Serial.print(newChar);
      if (!(newChar == 0x00 || newChar == 255 || i == 4095)) // End of configuration
      {
        if (i > 3 && newChar == CONFIGURATION_SEPARATOR)
        {
          separatorFound = true;
        }
        eepromData = eepromData + char(newChar);
        if (i == 3) // Read transmitter Id
        {
          byte* p = (byte*)(void*)&(config->transmitterId);
          unsigned int i;
          for (i = 0; i < 3; i++) {
            *p++ = (byte)eepromData.charAt(i);
            Serial.print("Char 2: ");
            Serial.print((byte)eepromData.charAt(i));
          }
          eepromData = ""; // Reset data
          Serial.print("Loaded config = ");
          Serial.print(config->transmitterId);
          Serial.print("\r\n");
        }
        if (separatorFound)
        {
          Serial.print("Separator found");
          if (!appEngineRead)
          {
            appEngineRead = true;
            config->appEngineAddress = eepromData;
          }
          else
          {
            if (readingSSID)
            {
              nextSSID = eepromData;
            }
            else
            {
              nextPassword = eepromData;
              WifiData newWifi = WifiData();
              newWifi.ssid = nextSSID;
              newWifi.password = nextPassword;
              config->wifiList->add(newWifi);
            }
            readingSSID = !readingSSID;
          }
          eepromData = ""; // Reset data to read
        }
      }
      else {
        continueReading = false;
      }
      
      i++;
    }
  /*}
  else
  {
    // Configuration is invalid
    
  }*/
  _loaded = true;
  return config;
}

void Configuration::SaveConfig() {
  if(!_loaded) {
    _bridgeConfig = LoadConfig();
  }
}
