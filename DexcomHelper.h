#ifndef DexcomHelper_h
#define DexcomHelper_h

#include "Arduino.h"

class DexcomHelper {
  public:
    void IntToCharArray(unsigned int value, char* result);
    uint32_t TransmitterIdCharacterNumber(char character);
    uint32_t DexcomAsciiToSrc(char* transmitterId);
    char* DexcomSrcToAscii(uint32_t src);
    
  private:
     static char SRC_NAME_TABLE[32];
};

#endif
