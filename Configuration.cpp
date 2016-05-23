/*
 * WebServer.c - Library for managing configuration reading and saving in the EEPROM memory
 */


/*
 * The data structure is as follow
 * 
 * 4 first char are for TransmitterId in uint32_t format
 * 
 * Next characters until '¬' character is the App engine address
 * 
 * Next configs are all separated by '¬'(0xAC) character and will finish with '·'(0xB7) character 
 * ex:
 * 2g1bmyaddress.appspot.com¬wifi1¬password1¬wifi2¬password2·
 * Wifi SSID ¬ Wifi Password ¬
 * Wifi SSID ¬ Wifi Password ¬
 * Wifi SSID ¬ Wifi Password ·
 */
 
#include "Configuration.h"


/*
 * This method will get the transmitter Id from the EEPROM
 */
uint32_t Configuration::getTransmitterId() {
  EEPROM.begin(4096); // Use maximum allowed size
  uint32_t transmitterId;
  EEPROM_readAnything(0, transmitterId);
  /*int addr = EEPROM.get(addr, myNewFloat);
  int addr2 = EEPROM.get(addr, myNewFloat);
  int addr3 = EEPROM.get(addr, myNewFloat);
  int addr4 = EEPROM.get(addr, myNewFloat);*/
}

