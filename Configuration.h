#ifndef Configuration_h
#define Configuration_h

#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"



class Configuration {
  public:
    void Testing();
    void setTransmitterId(uint32_t transmitterId);
    uint32_t getTransmitterId();
  private:
    
};

#endif
