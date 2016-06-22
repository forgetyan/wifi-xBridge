#ifndef Configuration_h
#define Configuration_h

#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "LinkedList.h"
#include "DexcomHelper.h"

struct WifiData {
  String ssid = "wifi-xBridge";
  String password = "";
};

struct BridgeConfig {
  bool isDebug = false;
  String debugAddress = "";
  uint32_t transmitterId = 0;
  String appEngineAddress = "";
  String hotSpotName = "wifi-xBridge";
  String hotSpotPassword = "";
  LinkedList<WifiData*> *wifiList = new LinkedList<WifiData*>();
};

class Configuration {
  public:
    Configuration();
    void Testing();
    void setTransmitterId(uint32_t transmitterId);
    void setAppEngineAddress(String address);
    void setDebugAddress(String address);
    void setIsDebug(bool isDebug);
    void setHotSpotName(String name);
    void setHotSpotPass(String pass);
    bool getIsDebug();
    void saveSSID(String ssidName, String ssidPassword);
    void deleteSSID(String ssidName);
    uint32_t getTransmitterId();
    String getAppEngineAddress();
    String getDebugAddress();
    String getHotSpotName();
    String getHotSpotPass();
    void SaveConfig();
    int getWifiCount();
    WifiData* getWifiData(int position);
  private:
    BridgeConfig* LoadConfig();
    void WriteEEPROM(int position, char data);
    void WriteStringToEEPROM(int position, String data);
    BridgeConfig* getBridgeConfig();
    bool _loaded;
    BridgeConfig *_bridgeConfig;
    static DexcomHelper _dexcomHelper;
};

#endif
