/*
 * WebServer.c - Library for managing configuration reading and saving in the EEPROM memory
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

/*
 * This method will save the transmitter Id to the EEPROM
 */
void Configuration::setTransmitterId(uint32_t transmitterId) {
  EEPROM_writeAnything(1, transmitterId);
  /*byte value = 65;
  EEPROM.write(1, value);
  EEPROM.commit();*/
}

/*
 * This method will get the transmitter Id from the EEPROM
 */
uint32_t Configuration::getTransmitterId() {
  uint32_t transmitterId;
  //byte value = EEPROM.read(1);
  //transmitterId = value;
  EEPROM_readAnything(1, transmitterId);
  return transmitterId;
}

/*
 * This method will get the Google App Engine Address from the EEPROM
 */
 /*
String Configuration::getAppEngineAddress() {
  String appEngineAddress;
  byte value = EEPROM.read(1);
  
  return transmitterId;
}*/
