#ifndef Configuration_h
#define Configuration_h

#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "LinkedList.h"
#include "DexcomHelper.h"

struct WifiData {
  String ssid;
  String password;
};

struct BridgeConfig {
  uint32_t transmitterId;
  String appEngineAddress;
  LinkedList<WifiData> *wifiList = new LinkedList<WifiData>();
};

class Configuration {
  public:
    Configuration();
    void Testing();
    void setTransmitterId(uint32_t transmitterId);
    uint32_t getTransmitterId();
    void setAppEngineAddress(String value);
    String getAppEngineAddress();
  private:
    BridgeConfig* LoadConfig();
    void SaveConfig();
    BridgeConfig* getBridgeConfig();
    bool _loaded;
    BridgeConfig *_bridgeConfig;
    static DexcomHelper _dexcomHelper;
};

#endif
