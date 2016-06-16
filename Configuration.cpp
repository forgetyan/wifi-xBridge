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
 * Next characters until next '¬' is the hotspot wifi name (Default is "wifi-xBridge")
 * Next characters until next '¬' is the wifi password (Default none)
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
 * Configuration::setTransmitterId
 * -------------------------------
 * This method will save the transmitter Id to the EEPROM
 */
void Configuration::setTransmitterId(uint32_t transmitterId) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  bridgeConfig->transmitterId = transmitterId;
}

/*
 * Configuration::setAppEngineAddress
 * ----------------------------------
 * This method will save the App Engine Address
 */
void Configuration::setAppEngineAddress(String address) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  bridgeConfig->appEngineAddress = address;
}

/*
 * Configuration::saveSSID
 * -----------------------
 * This method save a new SSID in EEPROM
 */
void Configuration::saveSSID(String ssidName, String ssidPassword) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  Serial.print("Save SSID\r\n");
  // Create wifi Data Object
  WifiData* wifiData = (WifiData*)calloc(1, sizeof(WifiData));
  wifiData->ssid = ssidName;
  wifiData->password = ssidPassword;
  // Add to saved wifi list
  bridgeConfig->wifiList->add(wifiData);
  Serial.print("New wifi ssid: ");
  Serial.print(wifiData->ssid);
  Serial.print("\r\nSize is now:");
  Serial.print(bridgeConfig->wifiList->size());
  Serial.print("\r\n");
}

/* 
 * Configuration::deleteSSID
 * -------------------------
 * This method will delete the specified ssid if found
 */
void Configuration::deleteSSID(String ssidName) {
  Serial.print("Delete SSID\r\n");
  BridgeConfig* bridgeConfig = getBridgeConfig();
  for(int i = bridgeConfig->wifiList->size() - 1; i >= 0; i--)
  {
    Serial.print("Check ssid no:");
    Serial.print(i);
    Serial.print("\r\n");
    WifiData* wifiData = Configuration::getWifiData(i);
    if(wifiData->ssid == ssidName) {
      Serial.print("Needs to be removed\r\n");
      // remove from the list
      bridgeConfig->wifiList->remove(i);
      Serial.print("Removed from list\r\n");
      free(wifiData);
    }
  }
}
/*
 * Configuration::getWifiData
 * --------------------------
 * This method will get the specified saved Wifi Data
 */
WifiData* Configuration::getWifiData(int position) {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  return bridgeConfig->wifiList->get(position);
}

/*
 * Configuration::getWifiCount
 * ---------------------------
 * This method will return the number of saved Wifi
 */
int Configuration::getWifiCount() {
  BridgeConfig* bridgeConfig = getBridgeConfig();
  LinkedList<WifiData*>* wifiList = bridgeConfig->wifiList;
  return bridgeConfig->wifiList->size();
}

/*
 * Configuration::getTransmitterId
 * -------------------------------
 * This method will get the transmitter Id from the EEPROM
 */
uint32_t Configuration::getTransmitterId() {
  uint32_t transmitterId;
  //byte value = EEPROM.read(1);
  //transmitterId = value;
  EEPROM_readAnything(1, transmitterId);
  BridgeConfig* bridgeConfig = getBridgeConfig();
  return bridgeConfig->transmitterId;
  /*
  uint32_t transmitterId;
  //byte value = EEPROM.read(1);
  //transmitterId = value;
  EEPROM_readAnything(1, transmitterId);
  return transmitterId;*/
}

/*
 * Configuration::getAppEngineAddress
 * ----------------------------------
 * This method will get the Google App Engine Address
 */
String Configuration::getAppEngineAddress() {
  BridgeConfig* bridgeConfig = getBridgeConfig();

  if(bridgeConfig->appEngineAddress.length() > 0)
  {
    return bridgeConfig->appEngineAddress;
  }
  else
  {
    return "";
  }
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
  /*LinkedList<WifiData> *test= new LinkedList<WifiData>();
  Serial.print( test->size());*/
  
  BridgeConfig* config = (BridgeConfig*)calloc(1, sizeof(BridgeConfig));// BridgeConfig();
  config->wifiList = new LinkedList<WifiData*>();
  String eepromData;
  bool continueReading = true;
  bool separatorFound = false;
  bool readingSSID = true;
  bool appEngineRead = false;
  bool hotspotNameRead = false;
  bool hotspotPasswordRead = false;
  char firstChar = EEPROM.read(0);
  String nextSSID = "";
  String nextPassword = "";
  if (firstChar == 182) { //'¶'
    Serial.print("Configuration Valid\r\n");
    // Configuration is valid
    // Read transmitter ID
  
  EEPROM_readAnything(1, config->transmitterId);
  //config->transmitterId = EEPROM.read(1) | ( (int)EEPROM.read(2) << 8 ) | ( (int)EEPROM.read(3) << 16 ) | ( (int)EEPROM.read(4) << 24 ); 
  /*Serial.print("Transmitter ID:");
  Serial.print(config->transmitterId);
  Serial.print("\r\n");
  for(int i = 0; i < 4; i++)
  {
    Serial.print(EEPROM.read(i+1));
    Serial.print("\r\n");
  }*/
  
  

    int i = 5;
    config->appEngineAddress = "";
    config->hotSpotName = "";
    config->hotSpotPassword = "";
    while(continueReading) {
      byte newChar = EEPROM.read(i);
      if (!(newChar == 0x00 || newChar == 255 || i == 4095)) // End of configuration
      {
        separatorFound = newChar == CONFIGURATION_SEPARATOR;
        
        if (separatorFound)
        {
          if (!appEngineRead)
          {
            appEngineRead = true;
            config->appEngineAddress = eepromData;
          }
          else if (!hotspotNameRead)
          {
            hotspotNameRead = true;
            config->hotSpotName = eepromData;
          }
          else if (!hotspotPasswordRead)
          {
            hotspotPasswordRead = true;
            config->hotSpotPassword = eepromData;
          }
          else // Everything else is saved wifi SSID and Passwords
          {
            if (readingSSID)
            {
              nextSSID = eepromData;
            }
            else
            {
              nextPassword = eepromData;
              WifiData* newWifi = (WifiData*)calloc(1, sizeof(WifiData));
              newWifi->ssid = nextSSID;
              newWifi->password = nextPassword;
              config->wifiList->add(newWifi);
            }
            readingSSID = !readingSSID;
          }
          eepromData = ""; // Reset data to read
        }
        else
        {
          eepromData = eepromData + char(newChar);
        }
      }
      else {
        continueReading = false;
      }
      
      i++;
    }
  }
  else
  {
    // Configuration is invalid
    config->appEngineAddress = "";
    config->hotSpotName = "";
    config->hotSpotPassword = "";
  }
  _loaded = true;
  return config;
}

/*
 * Configuration::SaveConfig
 * -------------------------
 * This method will save the Data back to the EEPROM
 */
void Configuration::SaveConfig() {
  BridgeConfig* bridgeConfig = Configuration::getBridgeConfig();
  int position;
  WriteEEPROM(0, '¶');
  uint32_t transmitterId = Configuration::getTransmitterId();
  /*byte transmitterIdByteArray[4];
  transmitterIdByteArray[0] = lowByte(transmitterId);
  transmitterIdByteArray[1] = lowByte(transmitterId >> 8);
  transmitterIdByteArray[2] = lowByte(transmitterId >> 16);
  transmitterIdByteArray[3] = lowByte(transmitterId >> 24);
  for(int i = 0; i < 4; i++)
  {
    Serial.print(transmitterIdByteArray[i]);
    Serial.print("\r\n");
  }*/
  /*Configuration::WriteEEPROM(1, transmitterIdByteArray[0]);
  Configuration::WriteEEPROM(2, transmitterIdByteArray[1]);
  Configuration::WriteEEPROM(3, transmitterIdByteArray[2]);
  Configuration::WriteEEPROM(4, transmitterIdByteArray[3]);*/
  EEPROM_writeAnything(1, transmitterId);
  position = 5;

  // Write App engine address
  if (_bridgeConfig->appEngineAddress.length() > 0) {
    Configuration::WriteStringToEEPROM(position, bridgeConfig->appEngineAddress);
    position = position + _bridgeConfig->appEngineAddress.length();
  }
  Configuration::WriteEEPROM(position , CONFIGURATION_SEPARATOR);
  position++;
  
  
  // now write hotspot wifi name
  if (_bridgeConfig->hotSpotName.length() > 0) {
    Configuration::WriteStringToEEPROM(position, bridgeConfig->hotSpotName);
    position = position + bridgeConfig->hotSpotName.length();
  }
  Configuration::WriteEEPROM(position , CONFIGURATION_SEPARATOR);
  position++;
  // now write hotspot wifi password
  if (_bridgeConfig->hotSpotPassword.length() > 0) {
    Configuration::WriteStringToEEPROM(position, bridgeConfig->hotSpotPassword);
    position = position + bridgeConfig->hotSpotPassword.length();
  }
  Configuration::WriteEEPROM(position , CONFIGURATION_SEPARATOR);
  position++;
  
  // Now write all saved wifi ssid and password
  
  int arrayLength = bridgeConfig->wifiList->size();
  for(int i = 0; i < arrayLength; i ++)
  {
    WifiData* wifiData = bridgeConfig->wifiList->get(i);
    if (wifiData->ssid.length() > 0) {
      WriteStringToEEPROM(position, wifiData->ssid);
      position = position + wifiData->ssid.length();
    }
    Configuration::WriteEEPROM(position, CONFIGURATION_SEPARATOR);
    position++;
    if (wifiData->password.length() > 0) {
      WriteStringToEEPROM(position, wifiData->password);
      position = position + wifiData->password.length();
    }
    Configuration::WriteEEPROM(position, CONFIGURATION_SEPARATOR);
    position++;
  }
  Configuration::WriteEEPROM(position , 255); // 255 character at the end
  delay(500); // EEPROM Save + Serial.print sometime generate garbage...
  EEPROM.commit();
  delay(500); // EEPROM Save + Serial.print sometime generate garbage...
  //_loaded = false;
  //free(bridgeConfig->wifiList);
  //free(bridgeConfig);
}

/*
 * Configuration::WriteStringToEEPROM
 * ----------------------------------
 * This method will save a string to the specified EEPROM position
 */
void Configuration::WriteStringToEEPROM(int position, String data)
{
  for(int i = 0; i < data.length(); i++){
    Configuration::WriteEEPROM(position + i, data.charAt(i));
  }
}

/*
 * Configuration::WriteEEPROM
 * --------------------------
 * This method will save a character to EEPROM if the current caracter is different at this position
 */
void Configuration::WriteEEPROM(int position, char data)
{
  //if(EEPROM.read(position) != data)
  //{
    EEPROM.write(position, data);
  //}
}

