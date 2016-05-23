#ifndef Configuration_h
#define Configuration_h

#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
//#include <../../../../libraries/EEPROM/EEPROM.h>



class Configuration {
  public:
    uint32_t getTransmitterId();
  private:
    
};

#endif
