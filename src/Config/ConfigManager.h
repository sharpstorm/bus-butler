#pragma once

#include "Preferences.h"

#define CONFIG_NAMESPACE "butler-config"

#define CONFIG_BUS_API_KEY "bus-api-key"
#define CONFIG_BRIGHTNESS "brightness"

class ConfigManager {
 private:
  Preferences preferences;

 public:
  ConfigManager();

  void saveLtaDatamallApiKey(String apiKey);
  String getLtaDatamallApiKey();

  void saveBrightness(uint8_t brightness);
  uint8_t getBrightness();
};

extern ConfigManager configManager;
