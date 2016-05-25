/*
 * DexcomHelper - Library for managing all helper methods for Dexcom
 */
#include "DexcomHelper.h"

 /* All possible values of Dexcom Transmitter ID Characters*/
char DexcomHelper::SRC_NAME_TABLE[32] = { '0', '1', '2', '3', '4', '5', '6', '7',
              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
              'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P',
              'Q', 'R', 'S', 'T', 'U', 'W', 'X', 'Y' };
/*
 * Function: DexcomSrcToAscii
 * -------------------------
 * Transform the uint32_t value of the Dexcom transmitter back to Ascii format
 * src: The special src value representing the Transmitter ID
 */
char* DexcomHelper::DexcomSrcToAscii(uint32_t src)
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
uint32_t DexcomHelper::DexcomAsciiToSrc(char* transmitterId)
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
uint32_t DexcomHelper::TransmitterIdCharacterNumber(char character)
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
void DexcomHelper::IntToCharArray(unsigned int value, char* result) {
  String textValue = String(value);
  textValue.toCharArray(result, 5);
}
