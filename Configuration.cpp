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
void setMyTransmitterId(uint32_t transmitterId) {
  EEPROM.begin(4096); // Use maximum allowed size
  EEPROM_writeAnything(0, transmitterId);
  EEPROM.end();
}

/*
 * This method will get the transmitter Id from the EEPROM
 */
uint32_t Configuration::getTransmitterId() {
  EEPROM.begin(4096); // Use maximum allowed size
  uint32_t transmitterId;
  EEPROM_readAnything(0, transmitterId);
  EEPROM.end();
  /*int addr = EEPROM.get(addr, myNewFloat);
  int addr2 = EEPROM.get(addr, myNewFloat);
  int addr3 = EEPROM.get(addr, myNewFloat);
  int addr4 = EEPROM.get(addr, myNewFloat);*/
}

